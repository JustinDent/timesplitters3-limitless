#ifndef TSFP_TEXTURE_PACK_H
#define TSFP_TEXTURE_PACK_H
struct D3D8Texture;
void tsfp_texture_upload(struct D3D8Texture *tex);
void tsfp_texture_release(struct D3D8Texture *tex);
#endif
