#ifndef vargfx_appvar_include_file
#define vargfx_appvar_include_file

#ifdef __cplusplus
extern "C" {
#endif

#define sizeof_global_palette 4
#define global_palette (vargfx_appvar[0])
#define vargfx_entries_num 1
extern unsigned char *vargfx_appvar[1];
unsigned char vargfx_init(void);

#ifdef __cplusplus
}
#endif

#endif
