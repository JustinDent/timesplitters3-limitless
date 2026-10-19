#ifndef TSFP_INTERPOLATION_DEPTH_H
#define TSFP_INTERPOLATION_DEPTH_H
/* Replay owns depth/stencil history. Keep one scratch surface per source
 * subresource used in the last replay, retaining its identity against reuse. */
#include "rhi.h"
#include <stdlib.h>
#include <string.h>
typedef struct TsfpReplayDepthEntry {
    struct TsfpReplayDepthEntry *next;
    RhiImage *source, *scratch;
    RhiView *view;
    uint32_t subresource, clear_flags;
    unsigned seen;
} TsfpReplayDepthEntry;
typedef struct {
    TsfpReplayDepthEntry *entries;
    unsigned epoch;
    int failed;
} TsfpReplayDepth;
static void tsfp_replay_depth_free(TsfpReplayDepthEntry *entry) {
    rhi_view_destroy(entry->view);
    rhi_image_destroy(entry->scratch);
    rhi_image_destroy(entry->source);
    free(entry);
}
static void tsfp_replay_depth_destroy(TsfpReplayDepth *cache) {
    while(cache->entries) {
        TsfpReplayDepthEntry *entry=cache->entries;
        cache->entries=entry->next;tsfp_replay_depth_free(entry);
    }
    cache->epoch=0;cache->failed=0;
}
static void tsfp_replay_depth_begin(TsfpReplayDepth *cache) {
    cache->failed=0;
    if(++cache->epoch==0) {
        for(TsfpReplayDepthEntry *e=cache->entries;e;e=e->next)e->seen=0;
        cache->epoch=1;
    }
}
static RhiView *tsfp_replay_depth_view(TsfpReplayDepth *cache,RhiImage *source,uint32_t subresource) {
    TsfpReplayDepthEntry *e;
    if(!source || cache->failed)return NULL;
    for(e=cache->entries;e;e=e->next)
        if(e->source==source && e->subresource==subresource)break;
    if(!e) {
        RhiImageDesc desc;memset(&desc,0,sizeof desc);rhi_image_get_desc(source,&desc);
        if(!cache->epoch || !desc.mip_levels || desc.mip_levels>32 || desc.type!=RHI_IMAGE_2D ||
           (uint64_t)subresource>=(uint64_t)desc.mip_levels*desc.depth || !(desc.bind&RHI_BIND_DEPTH)) {cache->failed=1;return NULL;}
        unsigned mip=subresource%desc.mip_levels;
        desc.width=desc.width>>mip;desc.height=desc.height>>mip;
        if(!desc.width)desc.width=1;if(!desc.height)desc.height=1;
        desc.type=RHI_IMAGE_2D;desc.depth=1;desc.mip_levels=1;desc.cube=0;
        desc.usage=RHI_USAGE_DEFAULT;desc.bind=RHI_BIND_DEPTH;desc.cpu_access=0;
        e=(TsfpReplayDepthEntry *)calloc(1,sizeof *e);
        if(!e){cache->failed=1;return NULL;}
        e->scratch=rhi_image_create(&desc,NULL);
        if(e->scratch)e->view=rhi_view_create(e->scratch,RHI_VIEW_DEPTH,NULL);
        if(!e->view){tsfp_replay_depth_free(e);cache->failed=1;return NULL;}
        e->source=rhi_image_retain(source);e->subresource=subresource;
        e->clear_flags=RHI_CLEAR_DEPTH | (desc.format==RHI_FORMAT_D24_UNORM_S8_UINT ? RHI_CLEAR_STENCIL : 0);
        e->next=cache->entries;cache->entries=e;
    }
    if(e->seen!=cache->epoch) {
        /* A frame with no depth clear must not inherit the previous guest or
         * replay depth. Later switches to this target retain this replay's writes. */
        rhi_clear_depth(e->view,e->clear_flags,1.0f,0);
        e->seen=cache->epoch;
    }
    return e->view;
}
/* Call after rebinding guest targets. Recycle unused entries instead of
 * accumulating every temporary depth target the title has ever created. */
static void tsfp_replay_depth_end(TsfpReplayDepth *cache) {
    TsfpReplayDepthEntry **link=&cache->entries;
    while(*link) {
        TsfpReplayDepthEntry *e=*link;
        if(e->seen!=cache->epoch){*link=e->next;tsfp_replay_depth_free(e);}
        else link=&e->next;
    }
}
#endif
