
typedef unsigned long  u64;
typedef unsigned int   u32;
typedef unsigned short u16;
typedef unsigned char  u8;
typedef long           i64;
typedef int            i32;
typedef short          i16;
typedef signed char    i8;

#ifdef __x86_64__
static i64 sys_write(int fd, const void *buf, u64 n) {
    i64 r;
    __asm__ volatile("syscall"
        : "=a"(r)
        : "0"(1), "D"((i64)fd), "S"(buf), "d"(n)
        : "rcx","r11","memory");
    return r;
}
static i64 sys_read(int fd, void *buf, u64 n) {
    i64 r;
    __asm__ volatile("syscall"
        : "=a"(r)
        : "0"(0), "D"((i64)fd), "S"(buf), "d"(n)
        : "rcx","r11","memory");
    return r;
}
static i64 sys_open(const char *path, int flags, int mode) {
    i64 r;
    __asm__ volatile("syscall"
        : "=a"(r)
        : "0"(2), "D"(path), "S"((i64)flags), "d"((i64)mode)
        : "rcx","r11","memory");
    return r;
}
static i64 sys_close(int fd) {
    i64 r;
    __asm__ volatile("syscall"
        : "=a"(r) : "0"(3), "D"((i64)fd)
        : "rcx","r11","memory");
    return r;
}
static i64 sys_lseek(int fd, i64 off, int whence) {
    i64 r;
    __asm__ volatile("syscall"
        : "=a"(r)
        : "0"(8), "D"((i64)fd), "S"(off), "d"((i64)whence)
        : "rcx","r11","memory");
    return r;
}
static void sys_exit(int code) {
    __asm__ volatile("syscall" :: "a"(60), "D"((i64)code));
    __builtin_unreachable();
}
#elif defined(__aarch64__)
static i64 sys_write(int fd, const void *buf, u64 n) {
    register i64 x0 __asm__("x0") = fd;
    register const void *x1 __asm__("x1") = buf;
    register u64 x2 __asm__("x2") = n;
    register i64 x8 __asm__("x8") = 64;
    __asm__ volatile("svc #0" : "=r"(x0) : "r"(x0),"r"(x1),"r"(x2),"r"(x8) : "memory");
    return x0;
}
static i64 sys_read(int fd, void *buf, u64 n) {
    register i64 x0 __asm__("x0") = fd;
    register void *x1 __asm__("x1") = buf;
    register u64 x2 __asm__("x2") = n;
    register i64 x8 __asm__("x8") = 63;
    __asm__ volatile("svc #0" : "=r"(x0) : "r"(x0),"r"(x1),"r"(x2),"r"(x8) : "memory");
    return x0;
}
static i64 sys_open(const char *path, int flags, int mode) {
    register i64 x0 __asm__("x0") = -100;
    register const char *x1 __asm__("x1") = path;
    register i64 x2 __asm__("x2") = flags;
    register i64 x3 __asm__("x3") = mode;
    register i64 x8 __asm__("x8") = 56;
    __asm__ volatile("svc #0" : "=r"(x0) : "r"(x0),"r"(x1),"r"(x2),"r"(x3),"r"(x8) : "memory");
    return x0;
}
static i64 sys_close(int fd) {
    register i64 x0 __asm__("x0") = fd;
    register i64 x8 __asm__("x8") = 57;
    __asm__ volatile("svc #0" : "=r"(x0) : "r"(x0),"r"(x8) : "memory");
    return x0;
}
static i64 sys_lseek(int fd, i64 off, int whence) {
    register i64 x0 __asm__("x0") = fd;
    register i64 x1 __asm__("x1") = off;
    register i64 x2 __asm__("x2") = whence;
    register i64 x8 __asm__("x8") = 62;
    __asm__ volatile("svc #0" : "=r"(x0) : "r"(x0),"r"(x1),"r"(x2),"r"(x8) : "memory");
    return x0;
}
static void sys_exit(int code) {
    register i64 x0 __asm__("x0") = code;
    register i64 x8 __asm__("x8") = 93;
    __asm__ volatile("svc #0" :: "r"(x0),"r"(x8));
    __builtin_unreachable();
}
#else
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
static i64 sys_write(int fd,const void*b,u64 n){return write(fd,b,n);}
static i64 sys_read(int fd,void*b,u64 n){return read(fd,b,n);}
static i64 sys_open(const char*p,int f,int m){return open(p,f,m);}
static i64 sys_close(int fd){return close(fd);}
static i64 sys_lseek(int fd,i64 o,int w){return lseek(fd,o,w);}
static void sys_exit(int c){exit(c);}
#endif


static u64 xstrlen(const char *s) {
    u64 n = 0; while(s[n]) n++; return n;
}
static void xmemcpy(void *d, const void *s, u64 n) {
    u8 *dd = d; const u8 *ss = s;
    while(n--) *dd++ = *ss++;
}
static void xmemset(void *d, u8 v, u64 n) {
    u8 *dd = d; while(n--) *dd++ = v;
}
static int xmemcmp(const void *a, const void *b, u64 n) {
    const u8 *aa=a, *bb=b;
    while(n--) { if(*aa != *bb) return (int)*aa-(int)*bb; aa++;bb++; }
    return 0;
}
static int xstrcmp(const char *a, const char *b) {
    while(*a && *a==*b){a++;b++;} return (u8)*a-(u8)*b;
}
static int xstrncmp(const char *a, const char *b, u64 n) {
    while(n-- && *a && *a==*b){a++;b++;} return n==(u64)-1?0:(u8)*a-(u8)*b;
}
static void xstrcpy(char *d, const char *s) {
    while((*d++=*s++));
}
static void xstrcat(char *d, const char *s) {
    while(*d)d++; while((*d++=*s++));
}

static void print_str(const char *s) { sys_write(1, s, xstrlen(s)); }
static void print_err(const char *s) { sys_write(2, s, xstrlen(s)); }

static void u64_to_hex(u64 v, char *buf, int width) {
    buf[width] = 0;
    for(int i = width-1; i >= 0; i--) {
        int d = v & 0xF;
        buf[i] = d < 10 ? '0'+d : 'a'+d-10;
        v >>= 4;
    }
}
static void u64_to_dec(u64 v, char *buf) {
    if(v == 0) { buf[0]='0'; buf[1]=0; return; }
    char tmp[24]; int n=0;
    while(v){ tmp[n++]='0'+(v%10); v/=10; }
    for(int i=0;i<n;i++) buf[i]=tmp[n-1-i];
    buf[n]=0;
}
static i64 parse_int(const char *s, int *consumed) {
    i64 sign = 1; int i = 0;
    if(s[i]=='-'){sign=-1;i++;}
    else if(s[i]=='+'){i++;}
    u64 val = 0;
    if(s[i]=='0' && (s[i+1]=='x'||s[i+1]=='X')) {
        i+=2;
        while((s[i]>='0'&&s[i]<='9')||(s[i]>='a'&&s[i]<='f')||(s[i]>='A'&&s[i]<='F')) {
            int d = s[i]>='a'?s[i]-'a'+10:s[i]>='A'?s[i]-'A'+10:s[i]-'0';
            val=(val<<4)|d; i++;
        }
    } else if(s[i]=='0' && (s[i+1]=='b'||s[i+1]=='B')) {
        i+=2;
        while(s[i]=='0'||s[i]=='1'){val=(val<<1)|(s[i]-'0');i++;}
    } else if(s[i]=='0' && s[i+1]>='0' && s[i+1]<='7') {
        i++;
        while(s[i]>='0'&&s[i]<='7'){val=(val<<3)|(s[i]-'0');i++;}
    } else {
        while(s[i]>='0'&&s[i]<='9'){val=val*10+(s[i]-'0');i++;}
    }
    if(consumed) *consumed = i;
    return sign*(i64)val;
}

#define ARENA_SIZE (64*1024*1024)
static u8 arena_mem[ARENA_SIZE];
static u64 arena_top = 0;

static void *arena_alloc(u64 size) {
    size = (size + 7) & ~7ULL;
    if(arena_top + size > ARENA_SIZE) {
        print_err("xasm: out of memory\n"); sys_exit(1);
    }
    void *p = arena_mem + arena_top;
    arena_top += size;
    xmemset(p, 0, size);
    return p;
}

#define O_RDONLY  0
#define O_WRONLY  1
#define O_CREAT   0100
#define O_TRUNC   01000
#define O_RDWR    2

static char *read_file(const char *path, u64 *out_size) {
    i64 fd = sys_open(path, O_RDONLY, 0);
    if(fd < 0) return 0;
    i64 size = sys_lseek((int)fd, 0, 2);
    sys_lseek((int)fd, 0, 0);
    char *buf = arena_alloc((u64)size + 1);
    i64 got = 0, rem = size;
    while(rem > 0) {
        i64 r = sys_read((int)fd, buf+got, (u64)rem);
        if(r <= 0) break;
        got += r; rem -= r;
    }
    buf[got] = 0;
    sys_close((int)fd);
    if(out_size) *out_size = (u64)got;
    return buf;
}

static int write_file(const char *path, const u8 *data, u64 size) {
    i64 fd = sys_open(path, O_WRONLY|O_CREAT|O_TRUNC, 0644);
    if(fd < 0) return -1;
    u64 written = 0;
    while(written < size) {
        i64 r = sys_write((int)fd, data+written, size-written);
        if(r <= 0) break;
        written += (u64)r;
    }
    sys_close((int)fd);
    return (written == size) ? 0 : -1;
}

#define INTERN_BUCKETS 4096
typedef struct InternEntry { const char *s; u64 len; struct InternEntry *next; } InternEntry;
static InternEntry *intern_table[INTERN_BUCKETS];

static u32 str_hash(const char *s, u64 len) {
    u32 h = 2166136261u;
    for(u64 i=0;i<len;i++) h=(h^(u8)s[i])*16777619u;
    return h;
}

static const char *intern(const char *s, u64 len) {
    u32 h = str_hash(s,len) & (INTERN_BUCKETS-1);
    for(InternEntry *e=intern_table[h];e;e=e->next)
        if(e->len==len && xmemcmp(e->s,s,len)==0) return e->s;
    InternEntry *e = arena_alloc(sizeof(InternEntry));
    char *copy = arena_alloc(len+1);
    xmemcpy(copy,s,len); copy[len]=0;
    e->s=copy; e->len=len;
    e->next=intern_table[h]; intern_table[h]=e;
    return copy;
}
static const char *intern_cstr(const char *s) { return intern(s,xstrlen(s)); }

#define MAX_DEFINES 512
#define MAX_INCLUDE_DEPTH 16

typedef struct { const char *name; const char *value; } Define;
static Define defines[MAX_DEFINES];
static int n_defines = 0;

static const char *define_get(const char *name) {
    for(int i=0;i<n_defines;i++)
        if(xstrcmp(defines[i].name,name)==0) return defines[i].value;
    return 0;
}
static void define_set(const char *name, const char *value) {
    for(int i=0;i<n_defines;i++)
        if(xstrcmp(defines[i].name,name)==0){defines[i].value=value;return;}
    if(n_defines>=MAX_DEFINES){print_err("too many defines\n");sys_exit(1);}
    defines[n_defines].name=name;
    defines[n_defines].value=value;
    n_defines++;
}

#define PP_BUF_MAX (32*1024*1024)
static char pp_buf[PP_BUF_MAX];
static u64  pp_pos = 0;

static void pp_emit(const char *s, u64 len) {
    if(pp_pos+len>=PP_BUF_MAX){print_err("preprocessor output too large\n");sys_exit(1);}
    xmemcpy(pp_buf+pp_pos, s, len);
    pp_pos += len;
}

static void preprocess_text(const char *src, const char *src_path, int depth);

static void skip_line(const char **p) {
    while(**p && **p!='\n') (*p)++;
    if(**p=='\n') (*p)++;
}

static int is_space(char c) { return c==' '||c=='\t'; }
static int is_alpha(char c) { return (c>='a'&&c<='z')||(c>='A'&&c<='Z')||c=='_'; }
static int is_alnum(char c) { return is_alpha(c)||(c>='0'&&c<='9'); }

static u64 read_ident(const char *s, char *out) {
    u64 i=0;
    while(is_alnum(s[i])){ out[i]=s[i]; i++; }
    out[i]=0; return i;
}

static void path_dir(const char *path, char *out) {
    i64 last=-1;
    for(i64 i=0;path[i];i++) if(path[i]=='/') last=i;
    if(last<0){out[0]='.';out[1]='/';out[2]=0;}
    else{xmemcpy(out,path,(u64)(last+1));out[last+1]=0;}
}

static void preprocess_text(const char *src, const char *src_path, int depth) {
    if(depth>MAX_INCLUDE_DEPTH){print_err("include depth exceeded\n");sys_exit(1);}

    int skip_stack[64]; int skip_depth=0; int skipping=0;
    const char *p = src;

    while(*p) {
        const char *line_start = p;

        while(*p && is_space(*p)) p++;

        if(*p=='#') {
            p++;
            while(is_space(*p)) p++;

            char directive[64]; u64 dlen=read_ident(p,directive);
            p+=dlen;
            while(is_space(*p)) p++;

            if(xstrcmp(directive,"include")==0) {
                if(skipping){skip_line(&p);continue;}
                char inc_path[512];
                char delim_end = 0;
                if(*p=='"'){p++;delim_end='"';}
                else if(*p=='<'){p++;delim_end='>';}
                else{print_err("bad include\n");sys_exit(1);}
                u64 pi=0;
                while(*p && *p!=delim_end && *p!='\n') inc_path[pi++]=*p++;
                inc_path[pi]=0;
                if(*p==delim_end) p++;
                skip_line(&p);
                char full[512];
                if(inc_path[0]=='/') {
                    xstrcpy(full, inc_path);
                } else {
                    char dir[512]; path_dir(src_path, dir);
                    xstrcpy(full,dir); xstrcat(full,inc_path);
                }
                char *inc_src = read_file(full, 0);
                if(!inc_src){ print_err("cannot open include: "); print_err(full); print_err("\n"); sys_exit(1);}
                preprocess_text(inc_src, full, depth+1);
                continue;

            } else if(xstrcmp(directive,"define")==0) {
                if(skipping){skip_line(&p);continue;}
                char name[128]; u64 nl=read_ident(p,name); p+=nl;
                while(is_space(*p)) p++;
                const char *val_start=p;
                while(*p && *p!='\n') p++;
                const char *val_end=p;
                while(val_end>val_start && is_space(*(val_end-1))) val_end--;
                u64 vlen=(u64)(val_end-val_start);
                char *val_copy = arena_alloc(vlen+1);
                xmemcpy(val_copy,val_start,vlen); val_copy[vlen]=0;
                define_set(intern_cstr(name), intern_cstr(val_copy));
                if(*p=='\n'){pp_emit("\n",1);p++;}
                continue;

            } else if(xstrcmp(directive,"undef")==0) {
                if(skipping){skip_line(&p);continue;}
                char name[128]; u64 nl=read_ident(p,name); p+=nl;
                for(int i=0;i<n_defines;i++)
                    if(xstrcmp(defines[i].name,name)==0){
                        defines[i]=defines[--n_defines]; break;
                    }
                skip_line(&p); continue;

            } else if(xstrcmp(directive,"ifdef")==0) {
                char name[128]; u64 nl=read_ident(p,name); p+=nl;
                skip_line(&p);
                int active = !skipping && (define_get(intern_cstr(name))!=0);
                skip_stack[skip_depth++] = skipping;
                skipping = !active;
                if(*p=='\n'){pp_emit("\n",1);}
                continue;

            } else if(xstrcmp(directive,"ifndef")==0) {
                char name[128]; u64 nl=read_ident(p,name); p+=nl;
                skip_line(&p);
                int active = !skipping && (define_get(intern_cstr(name))==0);
                skip_stack[skip_depth++] = skipping;
                skipping = !active;
                if(*p=='\n'){pp_emit("\n",1);}
                continue;

            } else if(xstrcmp(directive,"else")==0) {
                skip_line(&p);
                if(skip_depth>0) skipping = skip_stack[skip_depth-1] ? 1 : !skipping;
                continue;

            } else if(xstrcmp(directive,"endif")==0) {
                skip_line(&p);
                if(skip_depth>0) skipping=skip_stack[--skip_depth];
                continue;
            } else {
                if(!skipping) {
                    pp_emit(line_start,(u64)(p-line_start));
                }
                skip_line(&p);
                continue;
            }
        }

        if(skipping) { skip_line(&p); continue; }

        const char *lend = p;
        while(*lend && *lend!='\n') lend++;

        const char *q = line_start;
        while(q < lend) {
            if(is_alpha(*q)) {
                char tok[256]; u64 tl=0;
                while((is_alnum(*q))&&q<lend) tok[tl++]=*q++;
                tok[tl]=0;
                const char *repl = define_get(intern_cstr(tok));
                if(repl) pp_emit(repl,xstrlen(repl));
                else pp_emit(tok,tl);
            } else {
                pp_emit(q,1); q++;
            }
        }
        if(*lend=='\n'){pp_emit("\n",1); lend++;}
        p = lend;
    }
}


typedef enum {
    TK_EOF=0,
    TK_NEWLINE,
    TK_IDENT,
    TK_NUMBER,
    TK_STRING,
    TK_LBRACKET,
    TK_RBRACKET,
    TK_LPAREN,
    TK_RPAREN,
    TK_COMMA,
    TK_COLON,
    TK_PLUS,
    TK_MINUS,
    TK_STAR,
    TK_DOT,
    TK_AT,
} TKind;

typedef struct {
    TKind kind;
    const char *str;
    i64  num;
    int  line;
} Token;

#define MAX_TOKENS (1<<20)
static Token tokens[MAX_TOKENS];
static int n_tokens = 0;

static void lex(const char *src) {
    const char *p = src;
    int line = 1;

    while(*p) {
        while(*p && (*p==' '||*p=='\t'||*p=='\r')) p++;

        if(*p==';'||(*p=='/'&&p[1]=='/')) {
            while(*p && *p!='\n') p++;
            continue;
        }
        if(*p=='/'&&p[1]=='*') {
            p+=2;
            while(*p&&!(*p=='*'&&p[1]=='/')) { if(*p=='\n')line++; p++; }
            if(*p) p+=2;
            continue;
        }

        if(*p=='\n') {
            if(n_tokens>0 && tokens[n_tokens-1].kind!=TK_NEWLINE) {
                if(n_tokens>=MAX_TOKENS){print_err("too many tokens\n");sys_exit(1);}
                tokens[n_tokens++]=(Token){TK_NEWLINE,0,0,line};
            }
            line++; p++; continue;
        }

        if(*p==0) break;

        if(n_tokens>=MAX_TOKENS){print_err("too many tokens\n");sys_exit(1);}
        Token *t = &tokens[n_tokens++];
        t->line = line;

        if((*p>='0'&&*p<='9') ||
           ((*p=='-'||*p=='+') && (p[1]>='0'&&p[1]<='9') &&
            (n_tokens==0 || (tokens[n_tokens-1].kind!=TK_IDENT &&
                             tokens[n_tokens-1].kind!=TK_NUMBER &&
                             tokens[n_tokens-1].kind!=TK_RBRACKET &&
                             tokens[n_tokens-1].kind!=TK_RPAREN))))
        {
            int con=0;
            t->num=parse_int(p,&con);
            t->kind=TK_NUMBER;
            p+=con;
            continue;
        }
        if(is_alpha(*p)||*p=='.') {
            const char *start=p;
            while(is_alnum(*p)||*p=='.') p++;
            t->kind=TK_IDENT;
            t->str=intern(start,(u64)(p-start));
            continue;
        }

        if(*p=='"') {
            p++;
            const char *start=p;
            while(*p && *p!='"') p++;
            t->kind=TK_STRING;
            t->str=intern(start,(u64)(p-start));
            if(*p=='"') p++;
            continue;
        }

        switch(*p) {
            case '[': t->kind=TK_LBRACKET; break;
            case ']': t->kind=TK_RBRACKET; break;
            case '(': t->kind=TK_LPAREN;   break;
            case ')': t->kind=TK_RPAREN;   break;
            case ',': t->kind=TK_COMMA;    break;
            case ':': t->kind=TK_COLON;    break;
            case '+': t->kind=TK_PLUS;     break;
            case '-': t->kind=TK_MINUS;    break;
            case '*': t->kind=TK_STAR;     break;
            case '@': t->kind=TK_AT;       break;
            default:
                n_tokens--;
                break;
        }
        p++;
    }
    if(n_tokens==0 || tokens[n_tokens-1].kind!=TK_NEWLINE)
        tokens[n_tokens++]=(Token){TK_NEWLINE,0,0,line};
    tokens[n_tokens]=(Token){TK_EOF,0,0,line};
}


#define MAX_SYMBOLS 8192
typedef struct {
    const char *name;
    u64  addr;
    int  defined;
    int  is_res;
    u64  res_size;
} Symbol;
static Symbol symbols[MAX_SYMBOLS];
static int n_symbols = 0;

static Symbol *sym_find(const char *name) {
    for(int i=0;i<n_symbols;i++)
        if(xstrcmp(symbols[i].name,name)==0) return &symbols[i];
    return 0;
}
static Symbol *sym_get_or_create(const char *name) {
    Symbol *s=sym_find(name);
    if(s) return s;
    if(n_symbols>=MAX_SYMBOLS){print_err("too many symbols\n");sys_exit(1);}
    s=&symbols[n_symbols++];
    s->name=name; s->addr=0; s->defined=0; s->is_res=0; s->res_size=0;
    return s;
}

#define OUT_MAX (32*1024*1024)
static u8 out_buf[OUT_MAX];
static u64 out_pos = 0;

static void emit_byte(u8 b) {
    if(out_pos>=OUT_MAX){print_err("output too large\n");sys_exit(1);}
    out_buf[out_pos++]=b;
}
static void emit_u16(u16 v) { emit_byte(v&0xFF); emit_byte((v>>8)&0xFF); }
static void emit_u32(u32 v) { emit_byte(v&0xFF);emit_byte((v>>8)&0xFF);emit_byte((v>>16)&0xFF);emit_byte((v>>24)&0xFF); }
static void emit_u64(u64 v) { emit_u32((u32)v); emit_u32((u32)(v>>32)); }

static void patch_u32(u64 offset, u32 v) {
    out_buf[offset  ]= v     &0xFF;
    out_buf[offset+1]=(v>> 8)&0xFF;
    out_buf[offset+2]=(v>>16)&0xFF;
    out_buf[offset+3]=(v>>24)&0xFF;
}
static void patch_u64(u64 offset, u64 v) {
    patch_u32(offset,(u32)v); patch_u32(offset+4,(u32)(v>>32));
}


typedef enum {
    RELOC_X86_REL32,
    RELOC_X86_ABS32,
    RELOC_X86_ABS64,
    RELOC_RV_J20,
    RELOC_RV_B12,
    RELOC_RV_ABS32,
    RELOC_RV_HI20,
    RELOC_RV_LO12,
} RelocKind;

#define MAX_RELOCS 65536
typedef struct {
    u64        offset;
    const char *sym;
    RelocKind  kind;
    i64        addend;
    int        line;
} Reloc;
static Reloc relocs[MAX_RELOCS];
static int n_relocs = 0;

static void add_reloc(u64 offset, const char *sym, RelocKind kind, i64 addend, int line) {
    if(n_relocs>=MAX_RELOCS){print_err("too many relocs\n");sys_exit(1);}
    relocs[n_relocs++]=(Reloc){offset,sym,kind,addend,line};
}

typedef enum { ARCH_UNKNOWN, ARCH_X86_64, ARCH_RISCV64 } Arch;
static Arch g_arch = ARCH_UNKNOWN;


typedef enum {
    OP_NONE,
    OP_REG,
    OP_IMM,
    OP_SYM,
    OP_MEM,
    OP_MEMREG,
} OpKind;

typedef struct {
    OpKind kind;
    int    reg;
    i64    imm;
    const char *sym;
    i64    mem_offset;
    int    mem_base_reg;
    int    mem_index_reg;
    int    mem_scale;
} Operand;


typedef struct { const char *name; int idx; int size; } RegDef;

static const RegDef x86_regs[] = {

    {"rax",0,8},{"rcx",1,8},{"rdx",2,8},{"rbx",3,8},
    {"rsp",4,8},{"rbp",5,8},{"rsi",6,8},{"rdi",7,8},
    {"r8",8,8},{"r9",9,8},{"r10",10,8},{"r11",11,8},
    {"r12",12,8},{"r13",13,8},{"r14",14,8},{"r15",15,8},

    {"eax",0,4},{"ecx",1,4},{"edx",2,4},{"ebx",3,4},
    {"esp",4,4},{"ebp",5,4},{"esi",6,4},{"edi",7,4},
    {"r8d",8,4},{"r9d",9,4},{"r10d",10,4},{"r11d",11,4},
    {"r12d",12,4},{"r13d",13,4},{"r14d",14,4},{"r15d",15,4},

    {"ax",0,2},{"cx",1,2},{"dx",2,2},{"bx",3,2},
    {"sp",4,2},{"bp",5,2},{"si",6,2},{"di",7,2},

    {"al",0,1},{"cl",1,1},{"dl",2,1},{"bl",3,1},
    {"ah",4,1},{"ch",5,1},{"dh",6,1},{"bh",7,1},
    {"spl",4,1},{"bpl",5,1},{"sil",6,1},{"dil",7,1},
    {"r8b",8,1},{"r9b",9,1},{"r10b",10,1},{"r11b",11,1},
    {"r12b",12,1},{"r13b",13,1},{"r14b",14,1},{"r15b",15,1},
    {0,0,0}
};

static int x86_reg_idx(const char *name, int *size_out) {
    for(int i=0;x86_regs[i].name;i++)
        if(xstrcmp(x86_regs[i].name,name)==0){
            if(size_out)*size_out=x86_regs[i].size;
            return x86_regs[i].idx;
        }
    return -1;
}

static int x86_reg_size(const char *name) {
    int sz=0; x86_reg_idx(name,&sz); return sz;
}

static u8 x86_rex(int W, int R, int X, int B) {
    return (u8)(0x40 | (W<<3) | (R<<2) | (X<<1) | B);
}

static u8 x86_modrm(int mod, int reg, int rm) {
    return (u8)((mod<<6)|((reg&7)<<3)|(rm&7));
}

static u8 x86_sib(int scale, int idx, int base) {
    return (u8)((scale<<6)|((idx&7)<<3)|(base&7));
}

static void x86_emit_rex_rr(int W, int reg, int rm) {
    int R=(reg>>3)&1, B=(rm>>3)&1;
    if(W||R||B) emit_byte(x86_rex(W,R,0,B));
}

static const char *x86_cc_names[] = {
    "o","no","b","nb","e","ne","be","nbe",
    "s","ns","p","np","l","nl","le","nle",
    "z","nz","ae","c","nc","a",
    0
};
static int x86_cc_map[] = {
    0,1,2,3,4,5,6,7,
    8,9,10,11,12,13,14,15,
    4,5,3,2,3,7
};
static int x86_get_cc(const char *s) {
    for(int i=0;x86_cc_names[i];i++)
        if(xstrcmp(x86_cc_names[i],s)==0) return x86_cc_map[i];
    return -1;
}

static int tok_pos;

static Token *peek(int off) { return &tokens[tok_pos+off]; }
static Token *next_tok(void) { return &tokens[tok_pos++]; }
static void skip_newlines(void) { while(tokens[tok_pos].kind==TK_NEWLINE) tok_pos++; }

static Operand parse_operand_x86(void) {
    Operand op; xmemset(&op,0,sizeof(op)); op.kind=OP_NONE;
    op.mem_base_reg=-1; op.mem_index_reg=-1; op.mem_scale=1;

    Token *t = peek(0);
    if(t->kind==TK_NEWLINE||t->kind==TK_EOF||t->kind==TK_COMMA) return op;

    if(t->kind==TK_LBRACKET) {
        tok_pos++;
        op.kind=OP_MEM;
        Token *inner=peek(0);
        if(inner->kind==TK_IDENT) {
            int rsz=0;
            int ri=x86_reg_idx(inner->str,&rsz);
            if(ri>=0) {
                tok_pos++;
                op.mem_base_reg=ri;
                op.kind=OP_MEMREG;
                if(peek(0)->kind==TK_PLUS) {
                    tok_pos++;
                    if(peek(0)->kind==TK_IDENT) {
                        int ri2=x86_reg_idx(peek(0)->str,&rsz);
                        if(ri2>=0) {
                            tok_pos++;
                            op.mem_index_reg=ri2;
                            if(peek(0)->kind==TK_STAR) {
                                tok_pos++;
                                if(peek(0)->kind==TK_NUMBER){op.mem_scale=(int)peek(0)->num;tok_pos++;}
                            }
                            if(peek(0)->kind==TK_PLUS){tok_pos++;if(peek(0)->kind==TK_NUMBER){op.mem_offset=peek(0)->num;tok_pos++;}}
                        } else {
                            op.sym=peek(0)->str; tok_pos++;
                            op.mem_offset=0;
                        }
                    } else if(peek(0)->kind==TK_NUMBER){
                        op.mem_offset=peek(0)->num; tok_pos++;
                    }
                } else if(peek(0)->kind==TK_MINUS) {
                    tok_pos++;
                    if(peek(0)->kind==TK_NUMBER){op.mem_offset=-peek(0)->num;tok_pos++;}
                }
            } else {
                op.sym=inner->str; tok_pos++;
                op.mem_offset=0;
                op.mem_base_reg=-1;
                op.kind=OP_MEM;
                if(peek(0)->kind==TK_PLUS){
                    tok_pos++;
                    if(peek(0)->kind==TK_NUMBER){op.mem_offset=peek(0)->num;tok_pos++;}
                } else if(peek(0)->kind==TK_MINUS){
                    tok_pos++;
                    if(peek(0)->kind==TK_NUMBER){op.mem_offset=-peek(0)->num;tok_pos++;}
                }
            }
        } else if(inner->kind==TK_NUMBER) {
            op.kind=OP_MEM; op.mem_offset=inner->num; tok_pos++;
        }
        if(peek(0)->kind==TK_RBRACKET) tok_pos++;
        return op;
    }

    if(t->kind==TK_IDENT) {
        int rsz=0;
        int ri=x86_reg_idx(t->str,&rsz);
        if(ri>=0) {
            tok_pos++;
            op.kind=OP_REG; op.reg=ri;
            op.imm=rsz;
            return op;
        }
        op.kind=OP_SYM; op.sym=t->str; tok_pos++;
        if(peek(0)->kind==TK_PLUS){tok_pos++;if(peek(0)->kind==TK_NUMBER){op.imm=peek(0)->num;tok_pos++;}}
        return op;
    }

    if(t->kind==TK_NUMBER) {
        op.kind=OP_IMM; op.imm=t->num; tok_pos++;
        return op;
    }

    return op;
}

static void x86_emit_mem(Operand *mem, int reg_field) {
    if(mem->kind==OP_MEM && mem->sym) {
        emit_byte(x86_modrm(0,reg_field&7,5));
        add_reloc(out_pos, mem->sym, RELOC_X86_REL32, mem->mem_offset, 0);
        emit_u32(0);
        return;
    }
    int base=mem->mem_base_reg;
    int idx=mem->mem_index_reg;
    i64 disp=mem->mem_offset;
    {char _d[32]; u64_to_dec(disp<0?(u64)(-disp):(u64)disp,_d); sys_write(2,disp<0?(const char*)"-":(const char*)"+",1); sys_write(2,_d,xstrlen(_d)); sys_write(2,(const char*)"\n",1);}
    int need_sib = (idx>=0)||(base==4);

    int mod; int disp_size;
    if(disp==0 && base!=5 && base!=13) { mod=0; disp_size=0; }
    else if(disp>=-128&&disp<=127)      { mod=1; disp_size=1; }
    else                                 { mod=2; disp_size=4; }

    if(need_sib) {
        emit_byte(x86_modrm(mod,reg_field&7,4));
        int scale_enc=0;
        if(mem->mem_scale==2)scale_enc=1;
        else if(mem->mem_scale==4)scale_enc=2;
        else if(mem->mem_scale==8)scale_enc=3;
        int ib = (idx>=0)?idx:4;
        emit_byte(x86_sib(scale_enc,ib,base));
    } else {
        emit_byte(x86_modrm(mod,reg_field&7,base));
    }
    if(disp_size==1) emit_byte((u8)(i8)disp);
    else if(disp_size==4) emit_u32((u32)(i32)disp);
}

static int x86_assemble_insn(const char *mnem, int line) {
    Operand ops[3]; int nops=0;
    while(peek(0)->kind!=TK_NEWLINE && peek(0)->kind!=TK_EOF && nops<3) {
        if(nops>0) {
            if(peek(0)->kind!=TK_COMMA) break;
            tok_pos++;
        }
        ops[nops++]=parse_operand_x86();
    }

#define OP0 ops[0]
#define OP1 ops[1]
#define OP2 ops[2]
#define RSIZE(o) ((int)(o).imm)

    if(xstrcmp(mnem,"mov")==0 && nops==2) {
        if(OP0.kind==OP_REG && OP1.kind==OP_REG) {
            int sz=RSIZE(OP0);
            if(sz==8){
                x86_emit_rex_rr(1,OP1.reg,OP0.reg);
                emit_byte(0x89);
                emit_byte(x86_modrm(3,OP1.reg&7,OP0.reg&7));
            } else if(sz==4){
                if(OP1.reg>=8||OP0.reg>=8){
                    emit_byte(x86_rex(0,(OP1.reg>>3)&1,0,(OP0.reg>>3)&1));
                }
                emit_byte(0x89);
                emit_byte(x86_modrm(3,OP1.reg&7,OP0.reg&7));
            } else if(sz==2){
                emit_byte(0x66); emit_byte(0x89);
                emit_byte(x86_modrm(3,OP1.reg&7,OP0.reg&7));
            } else {
                if(OP1.reg>=4||OP0.reg>=4)
                    emit_byte(x86_rex(0,(OP1.reg>>3)&1,0,(OP0.reg>>3)&1));
                emit_byte(0x88);
                emit_byte(x86_modrm(3,OP1.reg&7,OP0.reg&7));
            }
            return 0;
        }
        if(OP0.kind==OP_REG && OP1.kind==OP_IMM) {
            int sz=RSIZE(OP0);
            if(sz==8) {
                int r=OP0.reg;
                if(r>=8) emit_byte(x86_rex(1,0,0,1));
                else     emit_byte(x86_rex(1,0,0,0));
                emit_byte(0xB8|(r&7));
                emit_u64((u64)OP1.imm);
            } else if(sz==4) {
                int r=OP0.reg;
                if(r>=8) emit_byte(x86_rex(0,0,0,1));
                emit_byte(0xB8|(r&7));
                emit_u32((u32)OP1.imm);
            } else if(sz==2) {
                emit_byte(0x66);
                emit_byte(0xB8|(OP0.reg&7));
                emit_u16((u16)OP1.imm);
            } else {
                if(OP0.reg>=4) emit_byte(x86_rex(0,0,0,(OP0.reg>>3)&1));
                emit_byte(0xB0|(OP0.reg&7));
                emit_byte((u8)OP1.imm);
            }
            return 0;
        }
        if(OP0.kind==OP_REG && OP1.kind==OP_SYM) {
            int r=OP0.reg;
            if(r>=8) emit_byte(x86_rex(1,0,0,1));
            else     emit_byte(x86_rex(1,0,0,0));
            emit_byte(0xB8|(r&7));
            add_reloc(out_pos, OP1.sym, RELOC_X86_ABS64, OP1.imm, line);
            emit_u64(0);
            return 0;
        }
        if((OP0.kind==OP_MEM||OP0.kind==OP_MEMREG) && OP1.kind==OP_REG) {
            int sz=RSIZE(OP1);
            if(sz==8){
                x86_emit_rex_rr(1,OP1.reg,OP0.mem_base_reg>=0?OP0.mem_base_reg:0);
                if(OP0.kind==OP_MEM&&OP0.sym){
                    emit_byte(0x89);
                    x86_emit_mem(&OP0,OP1.reg);
                } else {
                    emit_byte(0x89);
                    x86_emit_mem(&OP0,OP1.reg);
                }
            } else if(sz==4){
                if(OP1.reg>=8) emit_byte(x86_rex(0,(OP1.reg>>3)&1,0,(OP0.mem_base_reg>>3)&1));
                emit_byte(0x89); x86_emit_mem(&OP0,OP1.reg);
            } else if(sz==2){
                emit_byte(0x66); emit_byte(0x89); x86_emit_mem(&OP0,OP1.reg);
            } else {
                emit_byte(0x88); x86_emit_mem(&OP0,OP1.reg);
            }
            return 0;
        }
        if(OP0.kind==OP_REG && (OP1.kind==OP_MEM||OP1.kind==OP_MEMREG)) {
            int sz=RSIZE(OP0);
            if(sz==8){
                x86_emit_rex_rr(1,OP0.reg,OP1.mem_base_reg>=0?OP1.mem_base_reg:0);
                emit_byte(0x8B); x86_emit_mem(&OP1,OP0.reg);
            } else if(sz==4){
                if(OP0.reg>=8) emit_byte(x86_rex(0,(OP0.reg>>3)&1,0,0));
                emit_byte(0x8B); x86_emit_mem(&OP1,OP0.reg);
            } else if(sz==2){
                emit_byte(0x66); emit_byte(0x8B); x86_emit_mem(&OP1,OP0.reg);
            } else {
                emit_byte(0x8A); x86_emit_mem(&OP1,OP0.reg);
            }
            return 0;
        }
        if((OP0.kind==OP_MEM||OP0.kind==OP_MEMREG) && OP1.kind==OP_IMM) {
            emit_byte(x86_rex(1,0,0,OP0.mem_base_reg>=8?1:0));
            emit_byte(0xC7); x86_emit_mem(&OP0,0);
            emit_u32((u32)OP1.imm);
            return 0;
        }
        return -1;
    }

    if(xstrcmp(mnem,"movzx")==0 && nops==2) {
        if(OP0.kind==OP_REG && (OP1.kind==OP_MEM||OP1.kind==OP_MEMREG||OP1.kind==OP_REG)) {
            int dst_sz=RSIZE(OP0);
            int src_sz=(OP1.kind==OP_REG)?RSIZE(OP1):1;
            x86_emit_rex_rr(dst_sz==8?1:0, OP0.reg, OP1.kind==OP_REG?OP1.reg:0);
            emit_byte(0x0F);
            emit_byte(src_sz==1?0xB6:0xB7);
            if(OP1.kind==OP_REG) emit_byte(x86_modrm(3,OP0.reg&7,OP1.reg&7));
            else x86_emit_mem(&OP1,OP0.reg);
            return 0;
        }
    }

    if(xstrcmp(mnem,"lea")==0 && nops==2) {
        if(OP0.kind==OP_REG && (OP1.kind==OP_MEM||OP1.kind==OP_MEMREG)) {
            int sz=RSIZE(OP0);
            x86_emit_rex_rr(sz==8?1:0,OP0.reg,OP1.mem_base_reg>=0?OP1.mem_base_reg:0);
            emit_byte(0x8D); x86_emit_mem(&OP1,OP0.reg);
            return 0;
        }
    }

    if(xstrcmp(mnem,"push")==0 && nops==1) {
        if(OP0.kind==OP_REG) {
            int r=OP0.reg;
            if(r>=8) emit_byte(x86_rex(0,0,0,1));
            emit_byte(0x50|(r&7));
            return 0;
        }
        if(OP0.kind==OP_IMM) {
            if(OP0.imm>=-128&&OP0.imm<=127){emit_byte(0x6A);emit_byte((u8)(i8)OP0.imm);}
            else{emit_byte(0x68);emit_u32((u32)OP0.imm);}
            return 0;
        }
    }
    if(xstrcmp(mnem,"pop")==0 && nops==1) {
        if(OP0.kind==OP_REG) {
            int r=OP0.reg;
            if(r>=8) emit_byte(x86_rex(0,0,0,1));
            emit_byte(0x58|(r&7));
            return 0;
        }
    }

    typedef struct { const char *mn; u8 op_rr; u8 op_ri_ext; u8 op_ri8_ext; } AluOp;
    static const AluOp alu_ops[] = {
        {"add",0x01,0,0},{"or",0x09,1,1},{"and",0x21,4,4},
        {"sub",0x29,5,5},{"xor",0x31,6,6},{"cmp",0x39,7,7},{0,0,0,0}
    };
    for(const AluOp *ao=alu_ops;ao->mn;ao++) {
        if(xstrcmp(mnem,ao->mn)!=0) continue;
        if(nops==2) {
            if(OP0.kind==OP_REG && OP1.kind==OP_REG) {
                int sz=RSIZE(OP0);
                x86_emit_rex_rr(sz==8?1:0,OP1.reg,OP0.reg);
                emit_byte(sz==1?ao->op_rr-1:ao->op_rr);
                emit_byte(x86_modrm(3,OP1.reg&7,OP0.reg&7));
                return 0;
            }
            if(OP0.kind==OP_REG && OP1.kind==OP_IMM) {
                int sz=RSIZE(OP0);
                int r=OP0.reg;
                if(OP1.imm>=-128&&OP1.imm<=127) {
                    x86_emit_rex_rr(sz==8?1:0,ao->op_ri8_ext,r);
                    emit_byte(sz==1?0x80:0x83);
                    emit_byte(x86_modrm(3,ao->op_ri8_ext,r&7));
                    emit_byte((u8)(i8)OP1.imm);
                } else {
                    x86_emit_rex_rr(sz==8?1:0,ao->op_ri_ext,r);
                    emit_byte(sz==1?0x80:0x81);
                    emit_byte(x86_modrm(3,ao->op_ri_ext,r&7));
                    if(sz==2){emit_u16((u16)OP1.imm);}
                    else{emit_u32((u32)OP1.imm);}
                }
                return 0;
            }
            if((OP0.kind==OP_MEM||OP0.kind==OP_MEMREG) && OP1.kind==OP_REG) {
                int sz=RSIZE(OP1);
                x86_emit_rex_rr(sz==8?1:0,OP1.reg,OP0.mem_base_reg>=0?OP0.mem_base_reg:0);
                emit_byte(sz==1?ao->op_rr-1:ao->op_rr);
                x86_emit_mem(&OP0,OP1.reg);
                return 0;
            }
            if(OP0.kind==OP_REG && (OP1.kind==OP_MEM||OP1.kind==OP_MEMREG)) {
                int sz=RSIZE(OP0);
                x86_emit_rex_rr(sz==8?1:0,OP0.reg,OP1.mem_base_reg>=0?OP1.mem_base_reg:0);
                emit_byte(sz==1?(ao->op_rr+1):ao->op_rr|2);
                x86_emit_mem(&OP1,OP0.reg);
                return 0;
            }
        }
        return -1;
    }

    if(xstrcmp(mnem,"imul")==0) {
        if(nops==2 && OP0.kind==OP_REG && OP1.kind==OP_REG) {
            x86_emit_rex_rr(1,OP0.reg,OP1.reg);
            emit_byte(0x0F); emit_byte(0xAF);
            emit_byte(x86_modrm(3,OP0.reg&7,OP1.reg&7));
            return 0;
        }
        if(nops==3 && OP0.kind==OP_REG && OP1.kind==OP_REG && OP2.kind==OP_IMM) {
            if(OP2.imm>=-128&&OP2.imm<=127){
                x86_emit_rex_rr(1,OP0.reg,OP1.reg);
                emit_byte(0x6B);
                emit_byte(x86_modrm(3,OP0.reg&7,OP1.reg&7));
                emit_byte((u8)(i8)OP2.imm);
            } else {
                x86_emit_rex_rr(1,OP0.reg,OP1.reg);
                emit_byte(0x69);
                emit_byte(x86_modrm(3,OP0.reg&7,OP1.reg&7));
                emit_u32((u32)OP2.imm);
            }
            return 0;
        }
    }

    typedef struct{const char*mn;u8 ext;}UnaryOp;
    static const UnaryOp unary_ops[]={{"idiv",7},{"div",6},{"mul",4},{"neg",3},{"not",2},{"inc",0},{"dec",1},{0,0}};
    for(const UnaryOp*uo=unary_ops;uo->mn;uo++){
        if(xstrcmp(mnem,uo->mn)!=0)continue;
        if(nops==1&&OP0.kind==OP_REG){
            int sz=RSIZE(OP0);
            x86_emit_rex_rr(sz==8?1:0,uo->ext,OP0.reg);
            if(xstrcmp(mnem,"inc")==0||xstrcmp(mnem,"dec")==0)
                emit_byte(sz==1?0xFE:0xFF);
            else
                emit_byte(sz==1?0xF6:0xF7);
            emit_byte(x86_modrm(3,uo->ext,OP0.reg&7));
            return 0;
        }
    }

    typedef struct{const char*mn;u8 ext;}ShiftOp;
    static const ShiftOp shift_ops[]={{"shl",4},{"sal",4},{"shr",5},{"sar",7},{"rol",0},{"ror",1},{0,0}};
    for(const ShiftOp*so=shift_ops;so->mn;so++){
        if(xstrcmp(mnem,so->mn)!=0)continue;
        if(nops==2&&OP0.kind==OP_REG){
            int sz=RSIZE(OP0);
            if(OP1.kind==OP_IMM){
                x86_emit_rex_rr(sz==8?1:0,so->ext,OP0.reg);
                if(OP1.imm==1){emit_byte(sz==1?0xD0:0xD1);emit_byte(x86_modrm(3,so->ext,OP0.reg&7));}
                else{emit_byte(sz==1?0xC0:0xC1);emit_byte(x86_modrm(3,so->ext,OP0.reg&7));emit_byte((u8)OP1.imm);}
            } else if(OP1.kind==OP_REG&&OP1.reg==1){
                x86_emit_rex_rr(sz==8?1:0,so->ext,OP0.reg);
                emit_byte(sz==1?0xD2:0xD3);
                emit_byte(x86_modrm(3,so->ext,OP0.reg&7));
            }
            return 0;
        }
    }

    if(xstrcmp(mnem,"test")==0 && nops==2) {
        if(OP0.kind==OP_REG && OP1.kind==OP_REG) {
            int sz=RSIZE(OP0);
            x86_emit_rex_rr(sz==8?1:0,OP1.reg,OP0.reg);
            emit_byte(sz==1?0x84:0x85);
            emit_byte(x86_modrm(3,OP1.reg&7,OP0.reg&7));
            return 0;
        }
        if(OP0.kind==OP_REG && OP1.kind==OP_IMM) {
            int sz=RSIZE(OP0);
            x86_emit_rex_rr(sz==8?1:0,0,OP0.reg);
            emit_byte(sz==1?0xF6:0xF7);
            emit_byte(x86_modrm(3,0,OP0.reg&7));
            if(sz==8)emit_u32((u32)OP1.imm);
            else if(sz==4)emit_u32((u32)OP1.imm);
            else if(sz==2){emit_byte(0x66);emit_u16((u16)OP1.imm);}
            else emit_byte((u8)OP1.imm);
            return 0;
        }
    }

    if(xstrcmp(mnem,"jmp")==0 && nops==1) {
        if(OP0.kind==OP_REG) {
            int r=OP0.reg;
            if(r>=8) emit_byte(x86_rex(0,0,0,1));
            emit_byte(0xFF); emit_byte(x86_modrm(3,4,r&7));
            return 0;
        }
        if(OP0.kind==OP_SYM||OP0.kind==OP_IMM) {
            emit_byte(0xE9);
            if(OP0.kind==OP_SYM) add_reloc(out_pos,OP0.sym,RELOC_X86_REL32,0,line);
            emit_u32(0);
            return 0;
        }
    }

    if(xstrcmp(mnem,"call")==0 && nops==1) {
        if(OP0.kind==OP_REG){
            int r=OP0.reg;
            if(r>=8)emit_byte(x86_rex(0,0,0,1));
            emit_byte(0xFF); emit_byte(x86_modrm(3,2,r&7));
            return 0;
        }
        if(OP0.kind==OP_SYM||OP0.kind==OP_IMM) {
            emit_byte(0xE8);
            if(OP0.kind==OP_SYM) add_reloc(out_pos,OP0.sym,RELOC_X86_REL32,0,line);
            emit_u32(0);
            return 0;
        }
    }

    if(xstrcmp(mnem,"ret")==0) {
        if(nops==0){emit_byte(0xC3);return 0;}
        if(nops==1&&OP0.kind==OP_IMM){emit_byte(0xC2);emit_u16((u16)OP0.imm);return 0;}
    }


    if(mnem[0]=='j' && xstrlen(mnem)>=2) {
        int cc=x86_get_cc(mnem+1);
        if(cc>=0 && nops==1 && OP0.kind==OP_SYM) {
            emit_byte(0x0F); emit_byte((u8)(0x80|cc));
            add_reloc(out_pos,OP0.sym,RELOC_X86_REL32,0,line);
            emit_u32(0);
            return 0;
        }
    }

    if(xstrncmp(mnem,"cmov",4)==0) {
        int cc=x86_get_cc(mnem+4);
        if(cc>=0 && nops==2 && OP0.kind==OP_REG && OP1.kind==OP_REG) {
            x86_emit_rex_rr(1,OP0.reg,OP1.reg);
            emit_byte(0x0F); emit_byte((u8)(0x40|cc));
            emit_byte(x86_modrm(3,OP0.reg&7,OP1.reg&7));
            return 0;
        }
    }

    if(xstrncmp(mnem,"set",3)==0) {
        int cc=x86_get_cc(mnem+3);
        if(cc>=0 && nops==1 && OP0.kind==OP_REG) {
            if(OP0.reg>=4) emit_byte(x86_rex(0,0,0,(OP0.reg>>3)&1));
            emit_byte(0x0F); emit_byte((u8)(0x90|cc));
            emit_byte(x86_modrm(3,0,OP0.reg&7));
            return 0;
        }
    }

    if(xstrcmp(mnem,"nop")==0){emit_byte(0x90);return 0;}

    if(xstrcmp(mnem,"syscall")==0){emit_byte(0x0F);emit_byte(0x05);return 0;}

    if(xstrcmp(mnem,"int")==0 && nops==1 && OP0.kind==OP_IMM) {
        emit_byte(0xCD); emit_byte((u8)OP0.imm); return 0;
    }

    if(xstrcmp(mnem,"hlt")==0){emit_byte(0xF4);return 0;}
    if(xstrcmp(mnem,"cli")==0){emit_byte(0xFA);return 0;}
    if(xstrcmp(mnem,"sti")==0){emit_byte(0xFB);return 0;}
    if(xstrcmp(mnem,"cld")==0){emit_byte(0xFC);return 0;}
    if(xstrcmp(mnem,"std")==0){emit_byte(0xFD);return 0;}

    if(xstrcmp(mnem,"xchg")==0 && nops==2 && OP0.kind==OP_REG && OP1.kind==OP_REG) {
        x86_emit_rex_rr(1,OP0.reg,OP1.reg);
        if(OP0.reg==0||OP1.reg==0){
            int other=(OP0.reg==0)?OP1.reg:OP0.reg;
            emit_byte((u8)(0x90|(other&7)));
        } else {
            emit_byte(0x87);
            emit_byte(x86_modrm(3,OP0.reg&7,OP1.reg&7));
        }
        return 0;
    }

    if(xstrcmp(mnem,"cdqe")==0){emit_byte(0x48);emit_byte(0x98);return 0;}
    if(xstrcmp(mnem,"cdq")==0) {emit_byte(0x99);return 0;}
    if(xstrcmp(mnem,"cqo")==0) {emit_byte(0x48);emit_byte(0x99);return 0;}

    if(xstrcmp(mnem,"movsx")==0 && nops==2 && OP0.kind==OP_REG) {
        int dst_sz=RSIZE(OP0);
        int src_sz=(OP1.kind==OP_REG)?RSIZE(OP1):1;
        x86_emit_rex_rr(dst_sz==8?1:0,OP0.reg,OP1.kind==OP_REG?OP1.reg:0);
        emit_byte(0x0F);
        emit_byte(src_sz==1?0xBE:0xBF);
        if(OP1.kind==OP_REG) emit_byte(x86_modrm(3,OP0.reg&7,OP1.reg&7));
        else x86_emit_mem(&OP1,OP0.reg);
        return 0;
    }

    if(xstrcmp(mnem,"movsxd")==0 && nops==2 && OP0.kind==OP_REG && OP1.kind==OP_REG) {
        emit_byte(x86_rex(1,(OP0.reg>>3)&1,0,(OP1.reg>>3)&1));
        emit_byte(0x63);
        emit_byte(x86_modrm(3,OP0.reg&7,OP1.reg&7));
        return 0;
    }

#undef OP0
#undef OP1
#undef RSIZE
    return -1;
}

static const char *rv_reg_names_int[] = {
    "x0","x1","x2","x3","x4","x5","x6","x7",
    "x8","x9","x10","x11","x12","x13","x14","x15",
    "x16","x17","x18","x19","x20","x21","x22","x23",
    "x24","x25","x26","x27","x28","x29","x30","x31"
};
static const char *rv_reg_abi[] = {
    "zero","ra","sp","gp","tp","t0","t1","t2",
    "s0","s1","a0","a1","a2","a3","a4","a5",
    "a6","a7","s2","s3","s4","s5","s6","s7",
    "s8","s9","s10","s11","t3","t4","t5","t6"
};


static int rv_reg(const char *name) {
    if(xstrcmp(name,"fp")==0) return 8;
    for(int i=0;i<32;i++){
        if(xstrcmp(rv_reg_names_int[i],name)==0) return i;
        if(xstrcmp(rv_reg_abi[i],name)==0) return i;
    }
    return -1;
}

static u32 rv_R(u8 funct7,u8 rs2,u8 rs1,u8 funct3,u8 rd,u8 opcode){
    return ((u32)funct7<<25)|((u32)rs2<<20)|((u32)rs1<<15)|((u32)funct3<<12)|((u32)rd<<7)|(u32)opcode;
}
static u32 rv_I(i32 imm12,u8 rs1,u8 funct3,u8 rd,u8 opcode){
    return ((u32)(imm12&0xFFF)<<20)|((u32)rs1<<15)|((u32)funct3<<12)|((u32)rd<<7)|(u32)opcode;
}
static u32 rv_S(i32 imm12,u8 rs2,u8 rs1,u8 funct3,u8 opcode){
    u32 i11_5=(u32)((imm12>>5)&0x7F), i4_0=(u32)(imm12&0x1F);
    return (i11_5<<25)|((u32)rs2<<20)|((u32)rs1<<15)|((u32)funct3<<12)|(i4_0<<7)|(u32)opcode;
}
static u32 rv_B(i32 imm13,u8 rs2,u8 rs1,u8 funct3,u8 opcode){
    u32 i12=(u32)((imm13>>12)&1), i11=(u32)((imm13>>11)&1);
    u32 i10_5=(u32)((imm13>>5)&0x3F), i4_1=(u32)((imm13>>1)&0xF);
    return (i12<<31)|(i10_5<<25)|((u32)rs2<<20)|((u32)rs1<<15)|((u32)funct3<<12)|(i4_1<<8)|(i11<<7)|(u32)opcode;
}
static u32 rv_U(i32 imm20,u8 rd,u8 opcode){
    return ((u32)(imm20&0xFFFFF)<<12)|((u32)rd<<7)|(u32)opcode;
}
static u32 rv_J(i32 imm21,u8 rd,u8 opcode){
    u32 i20=(u32)((imm21>>20)&1), i19_12=(u32)((imm21>>12)&0xFF);
    u32 i11=(u32)((imm21>>11)&1), i10_1=(u32)((imm21>>1)&0x3FF);
    return (i20<<31)|(i10_1<<21)|(i11<<20)|(i19_12<<12)|((u32)rd<<7)|(u32)opcode;
}

static void rv_emit(u32 insn) {
    emit_byte(insn&0xFF); emit_byte((insn>>8)&0xFF);
    emit_byte((insn>>16)&0xFF); emit_byte((insn>>24)&0xFF);
}

typedef struct {
    OpKind kind;
    int reg;
    i64 imm;
    const char *sym;
    int base_reg;
} RVOp;

static void rv_skip_rparen(void){ if(peek(0)->kind==TK_RPAREN) tok_pos++; }

static RVOp rv_parse_op(void) {
    RVOp op; xmemset(&op,0,sizeof(op)); op.kind=OP_NONE; op.base_reg=-1;
    Token *t=peek(0);
    if(t->kind==TK_NEWLINE||t->kind==TK_EOF||t->kind==TK_COMMA) return op;

    if(t->kind==TK_NUMBER) {
        i64 off=t->num; tok_pos++;
        if(peek(0)->kind==TK_LPAREN) {
            tok_pos++;
            if(peek(0)->kind==TK_IDENT){
                int r=rv_reg(peek(0)->str);
                if(r>=0){tok_pos++; rv_skip_rparen(); op.kind=OP_MEMREG; op.base_reg=r; op.imm=off; return op;}
            }
        }
        op.kind=OP_IMM; op.imm=off; return op;
    }
    if(t->kind==TK_LPAREN){
        tok_pos++;
        if(peek(0)->kind==TK_IDENT){
            int r=rv_reg(peek(0)->str);
            if(r>=0){tok_pos++;rv_skip_rparen();}
            op.kind=OP_MEMREG; op.base_reg=r; op.imm=0; return op;
        }
    }

    if(t->kind==TK_IDENT) {
        int r=rv_reg(t->str);
        if(r>=0){tok_pos++; op.kind=OP_REG; op.reg=r; return op;}
        op.kind=OP_SYM; op.sym=t->str; tok_pos++;
        return op;
    }
    return op;
}

static int rv_assemble_insn(const char *mnem, int line) {
    RVOp ops[4]; int nops=0;
    while(peek(0)->kind!=TK_NEWLINE && peek(0)->kind!=TK_EOF && nops<4) {
        if(nops>0){if(peek(0)->kind!=TK_COMMA)break;tok_pos++;}
        ops[nops++]=rv_parse_op();
    }
#define RD ops[0].reg
#define RS1 ops[1].reg
#define RS2 ops[2].reg
#define IMM ops[1].imm

    typedef struct{const char*mn;u8 f3;u8 f7;} RvRAlu;
    static const RvRAlu rv_ralu[]={
        {"add",0,0},{"sub",0,0x20},{"sll",1,0},{"slt",2,0},{"sltu",3,0},
        {"xor",4,0},{"srl",5,0},{"sra",5,0x20},{"or",6,0},{"and",7,0},
        {"addw",0,0},{"subw",0,0x20},{"sllw",1,0},{"srlw",5,0},{"sraw",5,0x20},
        {"mul",0,1},{"mulh",1,1},{"mulhsu",2,1},{"mulhu",3,1},
        {"div",4,1},{"divu",5,1},{"rem",6,1},{"remu",7,1},
        {"mulw",0,1},{"divw",4,1},{"divuw",5,1},{"remw",6,1},{"remuw",7,1},
        {0,0,0}
    };
    for(const RvRAlu*a=rv_ralu;a->mn;a++){
        if(xstrcmp(mnem,a->mn)!=0)continue;
        if(nops==3&&ops[0].kind==OP_REG&&ops[1].kind==OP_REG&&ops[2].kind==OP_REG){
            u8 op=0x33;
            int mlen=(int)xstrlen(a->mn);
            if(a->mn[mlen-1]=='w'&&(a->f7==1||(a->f7==0||a->f7==0x20))) {
                if(mnem[mlen-1]=='w') op=0x3B;
            }
            rv_emit(rv_R(a->f7,(u8)ops[2].reg,(u8)ops[1].reg,a->f3,(u8)ops[0].reg,op));
            return 0;
        }
        return -1;
    }

    typedef struct{const char*mn;u8 f3;u8 opcode;}RvIAlu;
    static const RvIAlu rv_ialu[]={
        {"addi",0,0x13},{"slti",2,0x13},{"sltiu",3,0x13},
        {"xori",4,0x13},{"ori",6,0x13},{"andi",7,0x13},
        {"addiw",0,0x1B},
        {0,0,0}
    };
    for(const RvIAlu*a=rv_ialu;a->mn;a++){
        if(xstrcmp(mnem,a->mn)!=0)continue;
        if(nops>=2&&ops[0].kind==OP_REG&&ops[1].kind==OP_REG){
            i32 imm12=(nops>=3&&ops[2].kind==OP_IMM)?(i32)ops[2].imm:0;
            rv_emit(rv_I(imm12,(u8)RS1,a->f3,(u8)RD,a->opcode));
            return 0;
        }
        return -1;
    }

    typedef struct{const char*mn;u8 f3;u8 top;u8 op;}RvShift;
    static const RvShift rv_sh[]={
        {"slli",1,0,0x13},{"srli",5,0,0x13},{"srai",5,0x20,0x13},
        {"slliw",1,0,0x1B},{"srliw",5,0,0x1B},{"sraiw",5,0x20,0x1B},
        {0,0,0,0}
    };
    for(const RvShift*s=rv_sh;s->mn;s++){
        if(xstrcmp(mnem,s->mn)!=0)continue;
        if(nops==3&&ops[0].kind==OP_REG&&ops[1].kind==OP_REG&&ops[2].kind==OP_IMM){
            u32 shamt=(u32)ops[2].imm&0x3F;
            u32 i=((u32)s->top<<25)|(shamt<<20)|((u32)RS1<<15)|((u32)s->f3<<12)|((u32)RD<<7)|(u32)s->op;
            rv_emit(i); return 0;
        }
        return -1;
    }

    if(xstrcmp(mnem,"lui")==0 && nops==2 && ops[0].kind==OP_REG && ops[1].kind==OP_IMM){
        rv_emit(rv_U((i32)ops[1].imm,(u8)ops[0].reg,0x37)); return 0;
    }
    if(xstrcmp(mnem,"auipc")==0 && nops==2 && ops[0].kind==OP_REG && ops[1].kind==OP_IMM){
        rv_emit(rv_U((i32)ops[1].imm,(u8)ops[0].reg,0x17)); return 0;
    }
    if(xstrcmp(mnem,"lui")==0 && nops==2 && ops[0].kind==OP_REG && ops[1].kind==OP_SYM){
        add_reloc(out_pos, ops[1].sym, RELOC_RV_HI20, 0, line);
        rv_emit(rv_U(0,(u8)ops[0].reg,0x37)); return 0;
    }
    if(xstrcmp(mnem,"auipc")==0 && nops==2 && ops[0].kind==OP_REG && ops[1].kind==OP_SYM){
        add_reloc(out_pos, ops[1].sym, RELOC_RV_HI20, 0, line);
        rv_emit(rv_U(0,(u8)ops[0].reg,0x17)); return 0;
    }

    if(xstrcmp(mnem,"jal")==0) {
        if(nops==2 && ops[0].kind==OP_REG && ops[1].kind==OP_SYM){
            add_reloc(out_pos, ops[1].sym, RELOC_RV_J20, 0, line);
            rv_emit(rv_J(0,(u8)ops[0].reg,0x6F)); return 0;
        }
        if(nops==2 && ops[0].kind==OP_REG && ops[1].kind==OP_IMM){
            rv_emit(rv_J((i32)ops[1].imm,(u8)ops[0].reg,0x6F)); return 0;
        }
        if(nops==1 && ops[0].kind==OP_SYM){
            add_reloc(out_pos, ops[0].sym, RELOC_RV_J20, 0, line);
            rv_emit(rv_J(0,1,0x6F)); return 0;
        }
    }

    if(xstrcmp(mnem,"jalr")==0){
        if(nops==3&&ops[0].kind==OP_REG&&ops[1].kind==OP_REG&&ops[2].kind==OP_IMM){
            rv_emit(rv_I((i32)ops[2].imm,(u8)ops[1].reg,0,(u8)ops[0].reg,0x67)); return 0;
        }
        if(nops==2&&ops[0].kind==OP_REG&&ops[1].kind==OP_MEMREG){
            rv_emit(rv_I((i32)ops[1].imm,(u8)ops[1].base_reg,0,(u8)ops[0].reg,0x67)); return 0;
        }
        if(nops==1&&ops[0].kind==OP_REG){
            rv_emit(rv_I(0,(u8)ops[0].reg,0,0,0x67)); return 0;
        }
    }

    if(xstrcmp(mnem,"ret")==0&&nops==0){
        rv_emit(rv_I(0,1,0,0,0x67)); return 0;
    }

    if(xstrcmp(mnem,"j")==0&&nops==1&&ops[0].kind==OP_SYM){
        add_reloc(out_pos,ops[0].sym,RELOC_RV_J20,0,line);
        rv_emit(rv_J(0,0,0x6F)); return 0;
    }

    if(xstrcmp(mnem,"call")==0&&nops==1&&ops[0].kind==OP_SYM){
        add_reloc(out_pos,ops[0].sym,RELOC_RV_HI20,0,line);
        rv_emit(rv_U(0,1,0x17));
        add_reloc(out_pos,ops[0].sym,RELOC_RV_LO12,0,line);
        rv_emit(rv_I(0,1,0,1,0x67));
        return 0;
    }

    typedef struct{const char*mn;u8 f3;}RvBr;
    static const RvBr rv_br[]={
        {"beq",0},{"bne",1},{"blt",4},{"bge",5},{"bltu",6},{"bgeu",7},
        {"beqz",0},{"bnez",1},{"bltz",4},{"bgez",5},
        {0,0}
    };
    for(const RvBr*b=rv_br;b->mn;b++){
        if(xstrcmp(mnem,b->mn)!=0)continue;
        int is_pseudo=(xstrcmp(b->mn,"beqz")==0||xstrcmp(b->mn,"bnez")==0||xstrcmp(b->mn,"bltz")==0||xstrcmp(b->mn,"bgez")==0);
        if(is_pseudo&&nops==2&&ops[0].kind==OP_REG&&ops[1].kind==OP_SYM){
            add_reloc(out_pos,ops[1].sym,RELOC_RV_B12,0,line);
            rv_emit(rv_B(0,0,(u8)ops[0].reg,b->f3,0x63)); return 0;
        }
        if(nops==3&&ops[0].kind==OP_REG&&ops[1].kind==OP_REG&&ops[2].kind==OP_SYM){
            add_reloc(out_pos,ops[2].sym,RELOC_RV_B12,0,line);
            rv_emit(rv_B(0,(u8)ops[1].reg,(u8)ops[0].reg,b->f3,0x63)); return 0;
        }
        if(nops==3&&ops[0].kind==OP_REG&&ops[1].kind==OP_REG&&ops[2].kind==OP_IMM){
            rv_emit(rv_B((i32)ops[2].imm,(u8)ops[1].reg,(u8)ops[0].reg,b->f3,0x63)); return 0;
        }
        return -1;
    }

    typedef struct{const char*mn;u8 f3;}RvLoad;
    static const RvLoad rv_ld[]={
        {"lb",0},{"lh",1},{"lw",2},{"ld",3},{"lbu",4},{"lhu",5},{"lwu",6},
        {0,0}
    };
    for(const RvLoad*l=rv_ld;l->mn;l++){
        if(xstrcmp(mnem,l->mn)!=0)continue;
        if(nops==2&&ops[0].kind==OP_REG&&ops[1].kind==OP_MEMREG){
            rv_emit(rv_I((i32)ops[1].imm,(u8)ops[1].base_reg,l->f3,(u8)ops[0].reg,0x03));
            return 0;
        }
        if(nops==2&&ops[0].kind==OP_REG&&ops[1].kind==OP_SYM){
            add_reloc(out_pos,ops[1].sym,RELOC_RV_HI20,0,line);
            rv_emit(rv_U(0,(u8)ops[0].reg,0x17));
            add_reloc(out_pos,ops[1].sym,RELOC_RV_LO12,0,line);
            rv_emit(rv_I(0,(u8)ops[0].reg,l->f3,(u8)ops[0].reg,0x03));
            return 0;
        }
        return -1;
    }

    typedef struct{const char*mn;u8 f3;}RvStore;
    static const RvStore rv_st[]={
        {"sb",0},{"sh",1},{"sw",2},{"sd",3},
        {0,0}
    };
    for(const RvStore*s=rv_st;s->mn;s++){
        if(xstrcmp(mnem,s->mn)!=0)continue;
        if(nops==2&&ops[0].kind==OP_REG&&ops[1].kind==OP_MEMREG){
            rv_emit(rv_S((i32)ops[1].imm,(u8)ops[0].reg,(u8)ops[1].base_reg,s->f3,0x23));
            return 0;
        }
        return -1;
    }

    if(xstrcmp(mnem,"li")==0&&nops==2&&ops[0].kind==OP_REG){
        if(ops[1].kind==OP_IMM){
            i64 v=ops[1].imm;
            if(v>=-2048&&v<=2047){
                rv_emit(rv_I((i32)v,0,0,(u8)ops[0].reg,0x13));
            } else {
                i32 hi=(i32)((v+0x800)>>12), lo=(i32)(v-(i64)(hi<<12));
                rv_emit(rv_U(hi,(u8)ops[0].reg,0x37));
                if(lo) rv_emit(rv_I(lo,(u8)ops[0].reg,0,(u8)ops[0].reg,0x13));
            }
            return 0;
        }
        if(ops[1].kind==OP_SYM){
            add_reloc(out_pos,ops[1].sym,RELOC_RV_HI20,0,line);
            rv_emit(rv_U(0,(u8)ops[0].reg,0x37));
            add_reloc(out_pos,ops[1].sym,RELOC_RV_LO12,0,line);
            rv_emit(rv_I(0,(u8)ops[0].reg,0,(u8)ops[0].reg,0x13));
            return 0;
        }
    }

    if(xstrcmp(mnem,"la")==0&&nops==2&&ops[0].kind==OP_REG&&ops[1].kind==OP_SYM){
        add_reloc(out_pos,ops[1].sym,RELOC_RV_HI20,0,line);
        rv_emit(rv_U(0,(u8)ops[0].reg,0x17));
        add_reloc(out_pos,ops[1].sym,RELOC_RV_LO12,0,line);
        rv_emit(rv_I(0,(u8)ops[0].reg,0,(u8)ops[0].reg,0x13));
        return 0;
    }

    if(xstrcmp(mnem,"mv")==0&&nops==2&&ops[0].kind==OP_REG&&ops[1].kind==OP_REG){
        rv_emit(rv_I(0,(u8)ops[1].reg,0,(u8)ops[0].reg,0x13)); return 0;
    }

    if(xstrcmp(mnem,"nop")==0&&nops==0){
        rv_emit(rv_I(0,0,0,0,0x13)); return 0;
    }

    if(xstrcmp(mnem,"ecall")==0) { rv_emit(0x00000073); return 0; }
    if(xstrcmp(mnem,"ebreak")==0){ rv_emit(0x00100073); return 0; }


    if(xstrcmp(mnem,"not")==0&&nops==2&&ops[0].kind==OP_REG&&ops[1].kind==OP_REG){
        rv_emit(rv_I(-1,(u8)ops[1].reg,4,(u8)ops[0].reg,0x13)); return 0;
    }

    if(xstrcmp(mnem,"neg")==0&&nops==2&&ops[0].kind==OP_REG&&ops[1].kind==OP_REG){
        rv_emit(rv_R(0x20,(u8)ops[1].reg,0,0,(u8)ops[0].reg,0x33)); return 0;
    }

    if(xstrcmp(mnem,"seqz")==0&&nops==2&&ops[0].kind==OP_REG&&ops[1].kind==OP_REG){
        rv_emit(rv_I(1,(u8)ops[1].reg,3,(u8)ops[0].reg,0x13)); return 0;
    }

    if(xstrcmp(mnem,"snez")==0&&nops==2&&ops[0].kind==OP_REG&&ops[1].kind==OP_REG){
        rv_emit(rv_R(0,0,(u8)ops[1].reg,3,(u8)ops[0].reg,0x33)); return 0;
    }

    if(xstrcmp(mnem,"fence")==0){rv_emit(0x0FF0000F);return 0;}

    typedef struct{const char*mn;u8 f3;}RvCsr;
    static const RvCsr rv_csr[]={
        {"csrrw",1},{"csrrs",2},{"csrrc",3},{"csrrwi",5},{"csrrsi",6},{"csrrci",7},{0,0}
    };
    for(const RvCsr*c=rv_csr;c->mn;c++){
        if(xstrcmp(mnem,c->mn)!=0)continue;
        if(nops==3&&ops[0].kind==OP_REG&&ops[1].kind==OP_IMM){
            u32 csr=(u32)ops[1].imm&0xFFF;
            u32 src=(c->f3>=5)?(u32)ops[2].imm&0x1F:(u32)ops[2].reg;
            rv_emit((csr<<20)|(src<<15)|((u32)c->f3<<12)|((u32)ops[0].reg<<7)|0x73);
            return 0;
        }
    }

#undef RD
#undef RS1
#undef RS2
#undef IMM
    return -1;
}

typedef enum { PASS_SCAN=0, PASS_EMIT=1 } Pass;
static Pass current_pass;
static u64 load_address = 0;

static void handle_data_dir(const char *dir, int line) {
    (void)line;
    while(1) {
        Token *t=peek(0);
        if(t->kind==TK_NEWLINE||t->kind==TK_EOF) break;
        if(t->kind==TK_COMMA){tok_pos++;continue;}

        if(t->kind==TK_NUMBER){
            i64 v=t->num; tok_pos++;
            if(xstrcmp(dir,"db")==0)      emit_byte((u8)v);
            else if(xstrcmp(dir,"dw")==0) emit_u16((u16)v);
            else if(xstrcmp(dir,"dd")==0) emit_u32((u32)v);
            else if(xstrcmp(dir,"dq")==0) emit_u64((u64)v);
        } else if(t->kind==TK_STRING){
            const char *s=t->str; tok_pos++;
            if(xstrcmp(dir,"db")==0){
                while(*s){
                    u8 c;
                    if(*s=='\\'){
                        s++;
                        switch(*s){
                            case 'n':c='\n';break;case 't':c='\t';break;
                            case 'r':c='\r';break;case '0':c=0;break;
                            case '\\':c='\\';break;case '"':c='"';break;
                            default:c=(u8)*s;break;
                        }
                    } else c=(u8)*s;
                    emit_byte(c); s++;
                }
            }
        } else if(t->kind==TK_IDENT){
            const char *sym=t->str; tok_pos++;
            if(xstrcmp(dir,"dq")==0){
                add_reloc(out_pos,sym,RELOC_X86_ABS64,0,0);
                emit_u64(0);
            } else if(xstrcmp(dir,"dd")==0){
                add_reloc(out_pos,sym,RELOC_X86_ABS32,0,0);
                emit_u32(0);
            }
        } else { break; }
    }
}

static void assemble_pass(void) {
    tok_pos = 0;
    if(current_pass==PASS_EMIT){out_pos=0;}

    while(1){
        while(tokens[tok_pos].kind==TK_NEWLINE) tok_pos++;
        if(tokens[tok_pos].kind==TK_EOF) break;

        Token *t = &tokens[tok_pos];
        int line = t->line;

        if(t->kind!=TK_IDENT){tok_pos++;continue;}

        if(tokens[tok_pos+1].kind==TK_COLON) {
            const char *lname=t->str;
            tok_pos+=2;

            Symbol *sym=sym_get_or_create(lname);

            Token *next=&tokens[tok_pos];

            if(next->kind==TK_IDENT && (
                xstrcmp(next->str,"resb")==0||xstrcmp(next->str,"resw")==0||
                xstrcmp(next->str,"resd")==0||xstrcmp(next->str,"resq")==0))
            {
                const char *rsz=next->str; tok_pos++;
                u64 count=1;
                if(tokens[tok_pos].kind==TK_NUMBER){count=(u64)tokens[tok_pos].num;tok_pos++;}
                u64 unit=1;
                if(xstrcmp(rsz,"resw")==0)unit=2;
                else if(xstrcmp(rsz,"resd")==0)unit=4;
                else if(xstrcmp(rsz,"resq")==0)unit=8;
                u64 total=count*unit;

                if(current_pass==PASS_SCAN){
                    sym->addr=out_pos; sym->defined=1; sym->is_res=1; sym->res_size=total;
                } else {
                    sym->addr=out_pos;
                }
                for(u64 i=0;i<total;i++) emit_byte(0);
                continue;
            }

            if(next->kind==TK_IDENT && (
                xstrcmp(next->str,"db")==0||xstrcmp(next->str,"dw")==0||
                xstrcmp(next->str,"dd")==0||xstrcmp(next->str,"dq")==0))
            {
                const char *dir=next->str; tok_pos++;
                if(current_pass==PASS_SCAN){sym->addr=out_pos;sym->defined=1;}
                else {sym->addr=out_pos;}
                handle_data_dir(dir,line);
                continue;
            }

            if(current_pass==PASS_SCAN){sym->addr=out_pos;sym->defined=1;}
            else{sym->addr=out_pos;}
            if(tokens[tok_pos].kind==TK_NEWLINE||tokens[tok_pos].kind==TK_EOF) continue;
        }

        if(t->kind==TK_IDENT && tokens[tok_pos+1].kind!=TK_COLON) {
            const char *mnem=t->str; tok_pos++;

            if(xstrcmp(mnem,"db")==0||xstrcmp(mnem,"dw")==0||
               xstrcmp(mnem,"dd")==0||xstrcmp(mnem,"dq")==0) {
                handle_data_dir(mnem,line);
                continue;
            }
            if(xstrcmp(mnem,"times")==0){
                i64 count=0;
                if(tokens[tok_pos].kind==TK_NUMBER){count=tokens[tok_pos].num;tok_pos++;}
                if(tokens[tok_pos].kind==TK_IDENT){
                    const char *dir=tokens[tok_pos].str; tok_pos++;
                    for(i64 i=0;i<count;i++){
                        int saved=tok_pos;
                        handle_data_dir(dir,line);
                        if(i<count-1) tok_pos=saved;
                    }
                }
                continue;
            }
            if(xstrcmp(mnem,"align")==0){
                u64 al=16;
                if(tokens[tok_pos].kind==TK_NUMBER){al=(u64)tokens[tok_pos].num;tok_pos++;}
                while(out_pos%al) emit_byte(0x90);
                continue;
            }
            if(xstrcmp(mnem,"org")==0){
                if(tokens[tok_pos].kind==TK_NUMBER){
                    load_address=(u64)tokens[tok_pos].num; tok_pos++;
                }
                continue;
            }

            if(xstrcmp(mnem,"equ")==0){
                if(tokens[tok_pos].kind==TK_NUMBER) tok_pos++;
                continue;
            }

            if(current_pass==PASS_EMIT) {
                int r=-1;
                if(g_arch==ARCH_X86_64)   r=x86_assemble_insn(mnem,line);
                else if(g_arch==ARCH_RISCV64) r=rv_assemble_insn(mnem,line);
                if(r<0){
                    char lbuf[16]; u64_to_dec((u64)line,lbuf);
                    print_err("xasm: unknown instruction '");
                    print_err(mnem); print_err("' at line ");
                    print_err(lbuf); print_err("\n");
                }
            } else {
                int r=-1;
                u64 saved_pos=out_pos;
                if(g_arch==ARCH_X86_64)   r=x86_assemble_insn(mnem,line);
                else if(g_arch==ARCH_RISCV64) r=rv_assemble_insn(mnem,line);
                (void)r;(void)saved_pos;
            }
            while(tokens[tok_pos].kind!=TK_NEWLINE&&tokens[tok_pos].kind!=TK_EOF) tok_pos++;
            continue;
        }

        tok_pos++;
    }
}

static void resolve_relocs(void) {
    char errbuf[512];
    for(int i=0;i<n_relocs;i++){
        Reloc *r=&relocs[i];
        Symbol *sym=sym_find(r->sym);
        if(!sym||!sym->defined){
            print_err("xasm: undefined symbol '"); print_err(r->sym);
            print_err("'\n"); continue;
        }
        u64 sym_addr = sym->addr + load_address + (u64)r->addend;
        u64 pc_after;

        switch(r->kind){
        case RELOC_X86_REL32:
            pc_after = r->offset + 4;
            {
                i64 rel=(i64)(sym_addr-(pc_after+load_address));
                if(load_address==0) rel=(i64)(sym->addr+r->addend)-(i64)(r->offset+4);
                patch_u32(r->offset,(u32)(i32)rel);
            }
            break;
        case RELOC_X86_ABS32:
            patch_u32(r->offset,(u32)(sym_addr&0xFFFFFFFF));
            break;
        case RELOC_X86_ABS64:
            patch_u64(r->offset,sym_addr);
            break;
        case RELOC_RV_J20: {
            i64 rel;
            if(load_address==0) rel=(i64)(sym->addr+r->addend)-(i64)r->offset;
            else rel=(i64)(sym_addr)-(i64)(r->offset+load_address);
            u32 insn=*(u32*)(out_buf+r->offset);
            insn &= 0xFFF;
            i32 off=(i32)rel;
            u32 i20=(u32)((off>>20)&1), i19_12=(u32)((off>>12)&0xFF);
            u32 i11=(u32)((off>>11)&1), i10_1=(u32)((off>>1)&0x3FF);
            insn|=(i20<<31)|(i10_1<<21)|(i11<<20)|(i19_12<<12);
            patch_u32(r->offset,insn);
            break;
        }
        case RELOC_RV_B12: {
            i64 rel;
            if(load_address==0) rel=(i64)(sym->addr+r->addend)-(i64)r->offset;
            else rel=(i64)sym_addr-(i64)(r->offset+load_address);
            u32 insn=*(u32*)(out_buf+r->offset)&0x01FFF07F;
            i32 off=(i32)rel;
            u32 i12=(u32)((off>>12)&1),i11=(u32)((off>>11)&1);
            u32 i10_5=(u32)((off>>5)&0x3F),i4_1=(u32)((off>>1)&0xF);
            insn|=(i12<<31)|(i10_5<<25)|(i4_1<<8)|(i11<<7);
            patch_u32(r->offset,insn);
            break;
        }
        case RELOC_RV_HI20: {
            i64 abs;
            if(load_address==0) abs=(i64)(sym->addr+r->addend);
            else abs=(i64)sym_addr;
            i32 hi=(i32)(((u64)abs+0x800)>>12);
            u32 insn=*(u32*)(out_buf+r->offset)&0xFFF;
            insn|=(u32)(hi&0xFFFFF)<<12;
            patch_u32(r->offset,insn);
            break;
        }
        case RELOC_RV_LO12: {
            i64 abs;
            if(load_address==0) abs=(i64)(sym->addr+r->addend);
            else abs=(i64)sym_addr;
            i32 hi=(i32)(((u64)abs+0x800)>>12);
            i32 lo=(i32)(abs-(i64)(hi<<12));
            u32 insn=*(u32*)(out_buf+r->offset);
            insn=(insn&0x000FFFFF)|(((u32)(lo&0xFFF))<<20);
            patch_u32(r->offset,insn);
            break;
        }
        case RELOC_RV_ABS32: {
            i64 abs;
            if(load_address==0) abs=(i64)(sym->addr+r->addend);
            else abs=(i64)sym_addr;
            patch_u32(r->offset,(u32)abs);
            break;
        }
        }
        (void)errbuf;
    }
}

static void usage(void) {
    print_err(
        "xasm - Cross Assembler\n"
        "Usage: xasm -arch <arch> [options] <input.asm> -o <output.bin>\n"
        "\n"
        "Architectures:\n"
        "  x86_64    Intel/AMD 64-bit\n"
        "  riscv64   RISC-V RV64IMAC\n"
        "\n"
        "Options:\n"
        "  -arch <arch>   Target architecture (required)\n"
        "  -o <file>      Output file (default: out.bin)\n"
        "  -base <addr>   Load address in hex (default: 0)\n"
        "  -D NAME[=VAL]  Define preprocessor macro\n"
        "  -v             Verbose: print symbol table and stats\n"
        "\n"
        "Syntax:\n"
        "  label:              Define label\n"
        "  label: resb N      Reserve N bytes\n"
        "  label: db 0x41     Define byte data\n"
        "  [label+N]           Memory reference\n"
        "  #include \"f.asm\"   Include file\n"
        "  #define / #ifdef / #ifndef / #endif\n"
        "\n"
        "x86_64 registers: rax,rbx,...,r15  eax..r15d  ax..  al..\n"
        "riscv64 registers: x0-x31 or ABI: zero,ra,sp,gp,a0-a7,s0-s11,t0-t6\n"
    );
    sys_exit(1);
}

static const char *g_input = 0;
static const char *g_output = "out.bin";
static int g_verbose = 0;

static void parse_args(int argc, char **argv) {
    for(int i=1;i<argc;i++){
        if(xstrcmp(argv[i],"-arch")==0 && i+1<argc){
            i++;
            if(xstrcmp(argv[i],"x86_64")==0||xstrcmp(argv[i],"x86-64")==0||xstrcmp(argv[i],"amd64")==0)
                g_arch=ARCH_X86_64;
            else if(xstrcmp(argv[i],"riscv64")==0||xstrcmp(argv[i],"rv64")==0||xstrcmp(argv[i],"riscv")==0)
                g_arch=ARCH_RISCV64;
            else{print_err("xasm: unknown arch: ");print_err(argv[i]);print_err("\n");sys_exit(1);}
        } else if(xstrcmp(argv[i],"-o")==0 && i+1<argc){
            g_output=argv[++i];
        } else if(xstrcmp(argv[i],"-base")==0 && i+1<argc){
            load_address=(u64)parse_int(argv[++i],0);
        } else if(xstrncmp(argv[i],"-D",2)==0){
            const char *def=argv[i]+2;
            if(*def==0&&i+1<argc) def=argv[++i];
            char name[128]; int j=0;
            while(def[j]&&def[j]!='=') {name[j]=def[j];j++;} name[j]=0;
            const char *val=(def[j]=='=')?def+j+1:"1";
            define_set(intern_cstr(name),intern_cstr(val));
        } else if(xstrcmp(argv[i],"-v")==0){
            g_verbose=1;
        } else if(argv[i][0]=='-'){
            print_err("xasm: unknown option: ");print_err(argv[i]);print_err("\n");usage();
        } else {
            if(!g_input) g_input=argv[i];
            else{print_err("xasm: multiple input files not supported\n");sys_exit(1);}
        }
    }
    if(!g_input){print_err("xasm: no input file\n");usage();}
    if(g_arch==ARCH_UNKNOWN){print_err("xasm: -arch is required\n");usage();}
}

int main(int argc, char **argv) {
    parse_args(argc,argv);

    char *src=read_file(g_input,0);
    if(!src){print_err("xasm: cannot open '");print_err(g_input);print_err("'\n");sys_exit(1);}

    preprocess_text(src, g_input, 0);
    pp_buf[pp_pos]=0;

    lex(pp_buf);

    current_pass=PASS_SCAN;
    assemble_pass();

    n_relocs=0;
    current_pass=PASS_EMIT;
    out_pos=0;
    assemble_pass();

    resolve_relocs();

    if(write_file(g_output,out_buf,out_pos)<0){
        print_err("xasm: cannot write '");print_err(g_output);print_err("'\n");sys_exit(1);
    }

    if(g_verbose){
        char buf[32];
        print_str("xasm: output ");
        u64_to_dec(out_pos,buf); print_str(buf);
        print_str(" bytes -> "); print_str(g_output); print_str("\n");
        print_str("symbols:\n");
        for(int i=0;i<n_symbols;i++){
            print_str("  ");
            u64_to_hex(symbols[i].addr,buf,16);
            print_str("0x"); print_str(buf);
            print_str("  "); print_str(symbols[i].name);
            if(symbols[i].is_res){
                print_str(" (resv ");
                u64_to_dec(symbols[i].res_size,buf);
                print_str(buf); print_str("B)");
            }
            print_str("\n");
        }
    }

    sys_exit(0);
    return 0;
}
