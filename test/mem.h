#if defined(__linux__)
#  define LINUX 1
#elif defined(__APPLE__)
#  define APPLE 1
#elif defined(_WIN32)
#  define WINDOWS 1
#else // unknown platform
#  error "Unsupported platform!"
#endif // platform define

#define OS(x) (x)

////////////////////////////////////////////////////////////////////////
/// Declarations
////////////////////////////////////////////////////////////////////////

#include <limits>

namespace platform
{

    size_t memusedimpl(void);

} // namespace platform

////////////////////////////////////////////////////////////////////////
/// Linux implementation
////////////////////////////////////////////////////////////////////////

#if OS(LINUX)

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <cctype>

namespace platform
{

    static void * const LAST_ADDR = (void*)0xFFFFFFFFFFFFFFFF;

    struct mem_block
    {
        void * addr;
        size_t sz;
        bool   used;
    };
    
    class mem_block_iterator
    {
      public:
        mem_block_iterator() : fd_(open("/proc/self/maps", O_RDONLY)) { }
        
        ~mem_block_iterator() { if (fd_ != -1) { close(fd_); } }
        
        void * next(mem_block* pblk) const
        {
            if (fd_ == -1) { return LAST_ADDR; }
            /* at most 2 paths can be used per line */
            char buf[8192];
            int linelen = readline(buf);
            if (linelen == 0) { return LAST_ADDR; }
            /* parse line to mem block */
            /**
             * the format for a line is:
             * address                   perm fileoff  dev   inode                      path
             * 7f09a8fc6000-7f09a8fd2000 rw-p 00107000 00:38 32445                      location
             */
            int pos = 0;
            int nread = 0;
            size_t startval;
            /* read start addr */
            if ((nread = parsehex(buf + pos, startval))) {
                pos += nread;
                pblk->addr = reinterpret_cast<void*>(startval);
            } else {
                return LAST_ADDR;
            }
            
            if (buf[pos] != '-') { return LAST_ADDR; }
            ++pos;
            
            size_t endval;
            if ((nread = parsehex(buf + pos, endval))) {
                pos += nread;
                pblk->sz = (endval - startval);
            } else {
                return LAST_ADDR;
            }
            
            pos += skipws(buf + pos);
            
            /* now we are at the perms */
            if (pos + 3 >= linelen) { return LAST_ADDR; }
            
            if (buf[pos] == '-' && buf[pos+1] == '-' && buf[pos+2] == '-') {
                pblk->used = false;
            } else {
                pblk->used = true;
            }
            return reinterpret_cast<void*>(startval);
        }
      private:
        int fd_;
        
        int readline(char* buf) const
        {
            int total = 0;
            int nread;
            char ch;
            while(true) {
                nread = read(fd_, &ch, 1);
                if (nread <= 0) {
                    if (errno == EINTR) { continue; }
                    ch = '\0';
                    break;
                }
                ch = (ch == '\n') ? '\0' : ch;
                ++total;
                *buf++ = ch;
                if (ch == '\0') {
                    break;
                }
            }
            return total;
        }
        
        static int parsehex(const char* buf, size_t& value)
        {
            int nread = 0;
            value = 0;
            while (true) {
                char ch = ::tolower(*buf);
                ++nread;
                ++buf;
                if (ch >= '0' && ch <= '9') {
                    value = (value << 4) + (ch - '0');
                } else if (ch >= 'a' && ch <= 'f') {
                    value = (value << 4) + (ch - 'a' + 10);
                } else {
                    --nread;
                    break;
                }
            }
            return nread;
        }
        
        static int skipws(const char * buf)
        {
            int idx = 0;
            while (buf[idx] == ' ' || buf[idx] == '\t') { ++idx; }
            return idx;
        }
    };
    
    
    size_t memusedimpl(void)
    {
        mem_block_iterator it;
        mem_block blk;
        
        size_t mem = 0;
        
        while(it.next(&blk) != LAST_ADDR) {
            if (blk.used) {
                mem += blk.sz;
            }
        }
        
        return mem;
    }
    
} // namespace platform

#endif // linux implementation

////////////////////////////////////////////////////////////////////////
/// Apple implementation
////////////////////////////////////////////////////////////////////////

#if OS(APPLE)

#include <sys/sysctl.h>
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <mach/shared_region.h>

#include <unordered_set>

namespace platform
{

    inline bool issharedregion(mach_vm_address_t addr)
    {
        static cpu_type_t cputype;
        static bool gotcputype = false;
        if (!gotcputype) {
            size_t len = sizeof(cpu_type_t);
            sysctlbyname("sysctl.proc_cputype", &cputype, &len, NULL, 0);
            gotcputype = true;
        }
        
        if (cputype == CPU_TYPE_I386) {
            return addr >= SHARED_REGION_BASE_I386 &&
                addr < (SHARED_REGION_BASE_I386 + SHARED_REGION_SIZE_I386);
        } else if (cputype == CPU_TYPE_X86_64) {
            return addr >= SHARED_REGION_BASE_X86_64 &&
                addr < (SHARED_REGION_BASE_X86_64 + SHARED_REGION_SIZE_X86_64);
        } else {
            return false;
        }
    }
    
    size_t memusedimpl(void)
    {
        size_t committed = 0;
        
        task_t            self = mach_task_self();
        mach_vm_address_t addr = MACH_VM_MIN_ADDRESS;
        mach_vm_size_t    size = 0;
        
        vm_region_submap_info_64  info;
        mach_msg_type_number_t    count = VM_REGION_SUBMAP_INFO_COUNT_64;
        vm_region_info_t          infoptr = (vm_region_info_t)&info;
        natural_t                 nesting = 0;
        
        std::unordered_set<int> seen;
        
        while (true) {
            auto status = mach_vm_region_recurse(
                self,
                &addr,
                &size,
                &nesting,
                infoptr,
                &count);
            if (status != KERN_SUCCESS) {
                break;
            }
            
            if ((addr + size) > MACH_VM_MAX_ADDRESS) {
                break;
            }
            
            if (issharedregion(addr) && info.share_mode != SM_PRIVATE) {
                addr += size;
                continue;
            }
            
            if (info.is_submap) {
                ++nesting;
                continue;
            }
            
            if ((info.share_mode == SM_COW) && (info.ref_count == 1)) {
                info.share_mode = SM_PRIVATE;
            }
            
#define ALL_PAGES(x) (x.pages_resident + x.pages_shared_now_private + x.pages_swapped_out + x.pages_dirtied)
            
            if (((info.protection & VM_PROT_ALL) != VM_PROT_NONE) &&
                (info.share_mode != SM_EMPTY)) {
                if (info.share_mode == SM_SHARED) {
                    auto ret = seen.insert(info.object_id);
                    if (ret.second /* first occurrence */) {
                        committed += ALL_PAGES(info) * PAGE_SIZE;
                    }
                } else {
                    committed += ALL_PAGES(info) * PAGE_SIZE;
                }
            }
            
            addr += size;
        }
        
        return committed;
    }
    
} // namespace platform

#endif // apple implementation

////////////////////////////////////////////////////////////////////////
/// Windows implementation
////////////////////////////////////////////////////////////////////////

#if OS(WINDOWS)

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#  define NOMINMAX
#endif
#include <Windows.h>

namespace platform
{

    size_t memusedimpl(void)
    {
        size_t committed = 0;
        char*  p         = 0;
        char*  pnext     = 0;

        MEMORY_BASIC_INFORMATION mbi;

        do {

            p = pnext;

            VirtualQuery(p, &mbi, sizeof(mbi));

            if (mbi.State == MEM_COMMIT) {
                committed += mbi.RegionSize;
            }

            pnext = (char*)mbi.BaseAddress + mbi.RegionSize;

        } while (pnext > p);

        return committed;
    }

} // namespace platform

#endif // windows implementation

/**
 * Returns the current resident set size (physical memory use) measured
 * in bytes, or zero if the value cannot be determined on this OS.
 */
inline size_t getCurrentRSS( )
{
    return platform::memusedimpl();
}
