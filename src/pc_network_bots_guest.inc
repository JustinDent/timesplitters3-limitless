/* Host roster/activity prototype. Guest AI replication remains unfinished.
 * Called through a reproducible wrapper, never by editing generated game C. */
#ifdef _WIN32
#include <stdlib.h>
#include <stdio.h>
#include "pc_network_bots.h"
static unsigned pc_network_bots_option(const char *name,unsigned fallback,unsigned maximum)
{
    const char *text=getenv(name);char *end;unsigned long value;
    if (!text || !*text) return fallback;
    value=strtoul(text,&end,10);
    return *end || value>maximum ? fallback : (unsigned)value;
}
#endif
static void pc_network_bots_guest_config(void (*original)(void))
{
#ifdef _WIN32
    /* Both launch boundaries receive the final configuration by pointer. The
       network caller clears byte 2 AFTER sub_002D2C70, so patching that builder
       cannot survive until either the broadcast or the local load copy. */
    uint32_t address=*(uint32_t *)((uintptr_t)(g_esp+4u)+g_xbox_mem_offset);
    if (address==0x0078AD58u) {

    uint32_t active[16];unsigned i,count;

    static unsigned last_count,last_humans;
    uint8_t *config=(uint8_t *)((uintptr_t)0x0078AD58u+g_xbox_mem_offset);
    for (i=0;i<16u;i++)
        active[i]=*(uint32_t *)((uintptr_t)(0x0077728Cu+i*0x124u)+g_xbox_mem_offset);
    count=pc_network_bots_configure(config,PC_NETWORK_MATCH_BYTES,active,
        (const PcNetworkBot *)((uintptr_t)0x0075FD90u+g_xbox_mem_offset),
        pc_network_bots_option("TSFP_NETWORK_BOTS",0u,1u)==1u,
        pc_network_enabled() && *(uint32_t *)((uintptr_t)0x0076B108u+g_xbox_mem_offset)!=0,
        *(uint32_t *)((uintptr_t)0x0076B140u+g_xbox_mem_offset)==1u,
        *(uint32_t *)((uintptr_t)0x0078AD2Cu+g_xbox_mem_offset));
    if (count && (count!=last_count || config[0]!=last_humans))
        fprintf(stderr,"[NETWORK-BOT-TEST] host config: %u human slots, %u bots from Setup Bots; AI replication is experimental\n",
            (unsigned)config[0],count);
    last_count=count;last_humans=config[0];
    }
#endif
    original();
}

/* The stock activity predicate checks controller records, so it rejects every
 * AI slot. Only actor/HUD callers may accept an allocated bot: controller
 * enumeration and network player serializers must retain their human checks. */
static void pc_network_bots_guest_active(void (*original)(void))
{
#ifdef _WIN32
    const uint32_t caller=*(uint32_t *)((uintptr_t)g_esp+g_xbox_mem_offset);
    switch (caller) {
    case 0x000D8E5Du: /* actor update, including host-owned AI */
    case 0x00133BC9u: /* radar: actor pool, already includes human + AI slots */
    case 0x0013E0BBu: /* HUD character list: humans + AI, actor pool */
    case 0x0013E230u: /* HUD character draw: humans + AI, actor pool */
    case 0x0013EAFEu: /* HUD roster eligibility: humans + AI */
    case 0x0013EBDAu:
        if (pc_network_enabled() &&
            *(uint32_t *)((uintptr_t)0x0076B108u+g_xbox_mem_offset)) {
            const uint8_t *config=(const uint8_t *)((uintptr_t)0x007DE400u+g_xbox_mem_offset);
            const unsigned index=*(uint32_t *)((uintptr_t)(g_esp+4u)+g_xbox_mem_offset);
            const unsigned humans=config[0],bots=config[2];
            if (humans && bots && bots<=10u && humans+bots<=16u &&
                index>=humans && index<humans+bots &&
                *(uint32_t *)((uintptr_t)0x007356D8u+g_xbox_mem_offset)) {
                g_eax=1u;g_esp+=4u;return;
            }
        }
        break;
    default: break;
    }
#endif
    original();
}

#ifdef _WIN32
/* AI entities are not controller records. Give the original combat/event
 * serializers a separate, slot-based reference rather than a guest pointer
 * or an invalid entry in the human controller table. */
static uint32_t pc_network_bots_guest_entity(unsigned index)
{
    const uint8_t *config=(const uint8_t *)((uintptr_t)0x007DE400u+g_xbox_mem_offset);
    const unsigned humans=config[0],bots=config[2];
    uint32_t pool;
    if (!pc_network_enabled() || !*(uint32_t *)((uintptr_t)0x0076B108u+g_xbox_mem_offset) ||
        !humans || !bots || bots>10u || humans+bots>16u ||
        index<humans || index>=humans+bots) return 0;
    pool=*(uint32_t *)((uintptr_t)0x007356D8u+g_xbox_mem_offset);
    if (!pool) return 0;
    return *(uint32_t *)((uintptr_t)(pool+index*0xea0u)+g_xbox_mem_offset);
}
#endif
/* Combat first checks whether a victim can be encoded. Accept the same AI
 * slots as the codec, without inventing controller records or object IDs. */
static void pc_network_bots_guest_serializable(void (*original)(void))
{
#ifdef _WIN32
    const uint32_t entity=*(uint32_t *)((uintptr_t)(g_esp+4u)+g_xbox_mem_offset);
    unsigned index;
    if(entity) for(index=0;index<16u;index++) {
        if(pc_network_bots_guest_entity(index)==entity) {
            g_eax=1u;g_esp+=4u;return;
        }
    }
#endif
    original();
}
static void pc_network_bots_guest_encode(void (*original)(void))
{
#ifdef _WIN32
    unsigned index;
    if (g_ecx) for (index=0;index<16u;index++) {
        if (pc_network_bots_guest_entity(index)==g_ecx) {
            uint32_t *cursor=(uint32_t *)((uintptr_t)g_eax+g_xbox_mem_offset);
            uint8_t *bytes=(uint8_t *)((uintptr_t)*cursor+g_xbox_mem_offset);
            bytes[0]=4u;bytes[1]=(uint8_t)index;*cursor+=2u;
            g_esp+=4u;return;
        }
    }
#endif
    original();
}
static void pc_network_bots_guest_decode(void (*original)(void))
{
#ifdef _WIN32
    uint32_t *cursor=(uint32_t *)((uintptr_t)g_ecx+g_xbox_mem_offset);
    const uint8_t *bytes=(const uint8_t *)((uintptr_t)*cursor+g_xbox_mem_offset);
    if (bytes[0]==4u) {
        g_eax=pc_network_bots_guest_entity(bytes[1]);*cursor+=2u;
        g_esp+=4u;return;
    }
#endif
    original();
}

static void pc_network_bots_guest_frame(void (*original)(void))
{
    original();
#ifdef _WIN32
    static uint32_t last_tick;static int sent;
    const uint8_t *config=(const uint8_t *)((uintptr_t)0x007DE400u+g_xbox_mem_offset);
    unsigned role=*(uint32_t *)((uintptr_t)0x0076B140u+g_xbox_mem_offset);
    const unsigned humans=config[0],bots=config[2];
    unsigned index;uint32_t pool,host_ip=0,host_peer,tick,match;
    if(!pc_network_enabled() || *(uint32_t *)((uintptr_t)0x0076B1A4u+g_xbox_mem_offset)!=15u ||
       !humans || !bots || bots>10u || humans+bots>16u || (role!=1u && role!=2u)) {
        pc_network_bots_context(0,0,0,0,0);sent=0;return;
    }
    pool=*(uint32_t *)((uintptr_t)0x007356D8u+g_xbox_mem_offset);
    if(!pool) return;
    host_peer=*(uint32_t *)((uintptr_t)0x00774040u+g_xbox_mem_offset);
    if(role==2u) {
        if(host_peer>=16u) return;
        /* Original XNet sockaddr table, also used by sub_0002E530. The
           higher-level peer record contains a connection handle, not an IP. */
        host_ip=*(uint32_t *)((uintptr_t)(0x0066648Cu+host_peer*16u)+g_xbox_mem_offset);
    }
    match=config[0x99]|((uint32_t)config[0x54]<<8)|((uint32_t)config[0x55]<<16)|((uint32_t)config[0x56]<<24);
    tick=*(uint32_t *)((uintptr_t)0x007DE318u+g_xbox_mem_offset);
    pc_network_bots_context(role,host_ip,match,humans,bots);
    if(role==1u) {
        uint32_t addresses[16];unsigned count=0,peer;
        for(peer=0;peer<16u;peer++) {
            if(*(uint32_t *)((uintptr_t)(0x00666040u+peer*4u)+g_xbox_mem_offset))
                addresses[count++]=*(uint32_t *)((uintptr_t)(0x0066648Cu+peer*16u)+g_xbox_mem_offset);
        }
        pc_network_bots_targets(addresses,count);
    }
    for(index=humans;index<humans+bots;index++) {
        uint32_t entity=pc_network_bots_guest_entity(index);
        uint8_t *actor=(uint8_t *)((uintptr_t)(pool+index*0xea0u)+g_xbox_mem_offset);
        uint8_t *body=(uint8_t *)((uintptr_t)entity+g_xbox_mem_offset);
        PcNetworkBotState state;
        if(role==2u && pc_network_bots_receive(index,&state) &&
           pc_network_bot_state_valid(&state,humans,bots)) {
            if(!entity && state.health>0.f && (state.entity_flags&1u))
                entity=pc_network_bots_revive(index);
            if(!entity) continue;
            body=(uint8_t *)((uintptr_t)entity+g_xbox_mem_offset);
        } else if(role==2u || !entity) continue;
        if(role==1u) {
            uint32_t flags,life,dead;float health;
            memcpy(&flags,body+0x28,4);memcpy(&life,actor+0x18,4);
            memcpy(&dead,actor+0x2c,4);memcpy(&health,body+0x1cc,4);
            /* The network startup path leaves allocated AI bodies disabled.
               Activate living bots after their spawn state, never dead or
               inactive actors. Bit 0 enables drawing; bits 0x60 enable the original hit
               collision passes (including sub_0016CCD0). */
            if(life && life!=2u && !(dead&4u) && health>0.f && (flags&0x61u)!=0x61u) {
                flags|=0x61u;memcpy(body+0x28,&flags,4);
                pc_network_bots_entity_refresh(entity);
            }
            if(sent && tick-last_tick<3u) continue;
            memset(&state,0,sizeof state);state.slot=index;state.tick=tick;
            state.entity_flags=flags&0x61u;
            memcpy(state.flags,actor+0x24,12);memcpy(state.life,actor+0x10,16);
            memcpy(&state.animation,actor+0x94,4);memcpy(state.position,body+0x64,12);
            state.health=health;memcpy(&state.armor,body+0x1d0,4);
            memcpy(&state.last_damage_tick,body+0x1d4,4);memcpy(&state.max_health,body+0x1d8,4);
            memcpy(state.health_scale,actor+0xb08,8);
            memcpy(state.angles,body+0xc0,12);
            {
                uint32_t model;memcpy(&model,body+4,4);
                if(model) {
                    const uint8_t *pose=(const uint8_t *)((uintptr_t)model+g_xbox_mem_offset);
                    memcpy(&state.clip,pose+0xd4,4);memcpy(state.clip_channels,pose+0xdc,48);
                }
            }
            memcpy(state.scores,actor+0xa10,sizeof state.scores);
            pc_network_bots_send(&state);
        } else {
            uint32_t flags;
            memcpy(&flags,body+0x28,4);flags=(flags&~0x61u)|(state.entity_flags&0x61u);
            memcpy(body+0x28,&flags,4);memcpy(actor+0x24,state.flags,12);
            memcpy(actor+0x10,state.life,16);memcpy(actor+0x94,&state.animation,4);
            memcpy(body+0x64,state.position,12);memcpy(actor+0x30,state.position,12);
            memcpy(actor+0xbb4,state.position,12);memcpy(body+0x1cc,&state.health,4);
            memcpy(body+0x1d0,&state.armor,4);memcpy(body+0x1d4,&state.last_damage_tick,4);
            memcpy(body+0x1d8,&state.max_health,4);memcpy(actor+0xb08,state.health_scale,8);
            memcpy(body+0xc0,state.angles,12);
            memcpy(actor+0xa10,state.scores,sizeof state.scores);
            pc_network_bots_entity_refresh(entity);
            pc_network_bots_present(entity,state.clip,state.clip_channels);
        }
    }
    if(role==1u && (!sent || tick-last_tick>=3u)) {last_tick=tick;sent=1;}
#endif
}

/* Opt-in bounded diagnostic: record actual hit delivery, not health settings.
 * Tests route this file into their disposable run directory. */
static void pc_network_bots_guest_damage(void (*original)(void))
{
#ifdef _WIN32
    static unsigned traces;
    const char *path=getenv("TSFP_BOT_TRACE");
    if(path && *path && traces<256u && pc_network_enabled()) {
        uint32_t ctx=*(uint32_t *)((uintptr_t)(g_esp+4u)+g_xbox_mem_offset);
        uint32_t entity=*(uint32_t *)((uintptr_t)ctx+g_xbox_mem_offset);
        unsigned slot;
        for(slot=0;slot<16u;slot++) if(entity && pc_network_bots_guest_entity(slot)==entity) {
            float before,after,armor_before,armor_after,amount;FILE *log;uint32_t source;
            memcpy(&before,(void *)((uintptr_t)(entity+0x1ccu)+g_xbox_mem_offset),4);
            memcpy(&amount,(void *)((uintptr_t)(ctx+0x20u)+g_xbox_mem_offset),4);
            memcpy(&armor_before,(void *)((uintptr_t)(entity+0x1d0u)+g_xbox_mem_offset),4);
            source=*(uint32_t *)((uintptr_t)(ctx+4u)+g_xbox_mem_offset);
            original();
            memcpy(&after,(void *)((uintptr_t)(entity+0x1ccu)+g_xbox_mem_offset),4);
            memcpy(&armor_after,(void *)((uintptr_t)(entity+0x1d0u)+g_xbox_mem_offset),4);
            log=fopen(path,traces++?"a":"w");
            if(log) {fprintf(log,"bot=%u source=%08x damage=%g hp=%g->%g armor=%g->%g result=%u\n",slot,source,amount,before,after,armor_before,armor_after,g_eax);fclose(log);}
            return;
        }
    }
#endif
    original();
}
