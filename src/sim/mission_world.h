#ifndef FIST_SIM_MISSION_WORLD_H
#define FIST_SIM_MISSION_WORLD_H

#include "assets/klc.h"
#include "assets/orders.h"
#include "assets/units.h"
#include "sim/object_pool.h"
#include "sim/other_damage.h"
#include "sim/primary_fire.h"
#include "sim/projectile_flight.h"
#include "sim/projectile_launch.h"
#include "sim/random.h"
#include "sim/smoke.h"
#include "sim/tree.h"
#include "sim/vehicle_damage.h"
#include "sim/vehicle_state.h"
#include "sim/voice.h"

#include <stdint.h>

enum { FIST_MISSION_UNSUPPORTED = 2, FIST_MISSION_ARTILLERY_SIDE_SLOTS = 4 };

/* Owned common saved-format fields for static 16/25 and temporary 11/13.
 * Their later methods are distinct: readiness counts 16, samples 25 and
 * releases 11/13. Restoration does not substitute for those methods. */
typedef struct {
    fist_pool_allocation allocation;
    fist_object_pose pose;
    uint16_t projection_extent;
    uint16_t projection_scale;
    uint8_t flags;
    uint8_t secondary_flags;
    uint8_t ground_height;
    uint8_t variant;
} fist_saved_object_base;

/* The existing pool's physical type is the payload tag. Ground 0..3, other
 * 5/6/26/27, saved base 11/13/16/25, smoke 17/18, tree 21 and wreck 23 have
 * delivered typed restoration.
 * Primary launch installs dynamic shell 8 and muzzle 18. Canonical combat visits
 * publish explosion 4 and consume delivered death/effect/retirement classes.
 * Saved restoration of shell/explosion classes and living dispatch remain separate. */
typedef union {
    fist_vehicle_state vehicle;
    fist_other_actor other;
    fist_drifting_smoke smoke;
    fist_tree tree;
    fist_vehicle_wreck wreck;
    fist_projectile projectile;
    fist_muzzle_smoke muzzle;
    fist_explosion explosion;
    fist_saved_object_base saved_base;
} fist_mission_object;

typedef struct {
    uint16_t trees;
    uint16_t counted_static;
    uint16_t artillery_count[FIST_DAMAGE_SIDES];
    /* Actual runtime identities, in original registry order; no saved near
     * pointer translation. After preparation, entries beyond each count are NO_SLOT. */
    fist_pool_allocation artillery[FIST_DAMAGE_SIDES][FIST_MISSION_ARTILLERY_SIDE_SLOTS];
    uint8_t prepared;
} fist_mission_preparation;

/* Selected target message producer: typed class/side/variant, never a guest
 * text address. duration is the original shared display countdown. */
typedef struct {
    uint16_t duration;
    uint16_t type;
    uint8_t variant;
    bool enemy;
} fist_target_notice;

typedef struct {
    fist_object_pool pool;
    fist_random random;
    /* Canonical mission order owner. Saved-object-only installation leaves this
     * explicitly unloaded; command dispatch requires complete mission input. */
    fist_mission_orders orders;
    uint8_t orders_loaded;
    fist_mission_object objects[FIST_UNIT_REGISTRY_COUNT];
    /* Owns the sole physical roster, including overwritten registry orphans.
     * Reset/import do not configure mission combat factors/census/player UI. */
    fist_combat_state combat;
    /* Scheduler handoff after selected fatal damage; not an original record. */
    uint16_t pending_player_impact;
    fist_mission_preparation preparation;
    fist_voice_history voice;
    fist_target_notice target_notice;
} fist_mission_world;

/* Borrowed read-only physical projection shared by collision and target search.
 * Ground pose uses caller storage; other poses refer to canonical payloads. */
typedef struct {
    const fist_object_pose *pose;
    uint16_t projection_scale;
    uint8_t flags;
    uint8_t secondary_flags;
    uint8_t mode;
} fist_mission_view;

int fist_mission_world_view(const fist_mission_world *world, uint16_t slot,
                            fist_object_pose *ground_pose, fist_mission_view *out);

typedef struct {
    uint16_t slot;
    uint16_t tick;
    uint16_t voice_gate;
    uint8_t link_mode;
    bool coarse;
} fist_target_discovery_request;

typedef struct {
    fist_object_reference primary;
    fist_object_reference secondary;
    uint16_t primary_range;
    uint16_t secondary_operand;
    uint8_t priority;
    uint8_t count;
    fist_voice_request voice;
} fist_target_discovery_result;

/* Complete b011 using prepared canonical state. Preserve original preference,
 * range, secondary and old-target rules; invalidate stale runtime targets.
 * Saved near words are never interpreted. No RNG, parent dispatch or PCM.
 * Return 0, or -1 for invalid state/input, including a missing/mistagged projection;
 * all failures preserve world/output, including notification history. */
int fist_mission_world_discover_targets(fist_mission_world *world, const fist_klc_image *height,
                                        fist_target_discovery_request request,
                                        fist_target_discovery_result *out);

/* Actual shared e21c class/variant aim offsets. Inputs are canonical physical
 * projections; outputs own their poses. No visibility, references or mutation. */
int fist_mission_world_target_positions(const fist_mission_world *world, uint16_t actor,
                                        uint16_t target, fist_object_pose *source,
                                        fist_object_pose *destination);

typedef struct {
    uint16_t slot;
    fist_object_reference candidate;
    uint16_t tick;
    uint16_t voice_gate;
    bool automatic;
} fist_target_acquisition_request;

typedef struct {
    bool attempted;
    bool installed;
    bool message;
    fist_voice_request voice;
} fist_target_acquisition_result;

/* Complete ae32/a6e3 from prepared canonical orders/RNG/lifetimes. Direct
 * rejects preserve a valid old target; attempted invisible candidates clear it.
 * Automatic admission sets bit 128 even after a failed attempt. Stale references
 * cannot address successors. Selected live type-26 modes outside the authored
 * four-message domain fail atomically rather than read adjacent text. Unused
 * behavior/height/variant inputs do not restrict original early branches.
 * All failures preserve complete world/output; no parent, aiming or PCM. */
int fist_mission_world_acquire_target(fist_mission_world *world, const fist_klc_image *height,
                                      fist_target_acquisition_request request,
                                      fist_target_acquisition_result *out);

/* Complete f69:abd5/a18e target geometry. Produce +9b heading, existing +38
 * turret elevation and packed +99 range; retain all on null/lost target except
 * invalidating its runtime reference. No RNG, throttle or parent dispatch.
 * Invalid used projection/state fails atomically. */
int fist_mission_world_aim_target(fist_mission_world *world, uint16_t slot, bool coarse);

void fist_mission_world_reset(fist_mission_world *world);

/* Complete normal-side d84a/43c1 saved-object installation for delivered
 * classes, using decoded immutable definitions. Initialize only participating
 * ground actors, in input order; retain final RNG and all physical orphans.
 * Input units/random are borrowed; no input views survive. Units stay unchanged.
 * Random stays unchanged unless it aliases out->random; that supported reload
 * replaces the old world using its current RNG as the new initial state.
 * Returns OK, UNAVAILABLE for physical exhaustion, UNSUPPORTED for an undelivered
 * type, or -1 for invalid data/allocation. Every failure preserves out. This
 * owns neither terrain/contact installation nor living class dispatch/devices. */
int fist_mission_world_initialize(const fist_units *units, const fist_random *random,
                                  uint8_t link_mode, fist_mission_world *out);

/* Original mission-start census/list zeros followed by complete d755 dispatch
 * for every delivered current binding. Visit registry order, including deleted
 * nonparticipants; preserve physical orphans. Reset ground fields, initialize
 * tree/target/artillery preparation, sample actual op-54 heights and release
 * temporary objects. Height is borrowed; no views survive. Reuses class defaults,
 * canonical RNG and pool release. Repeated preparation consumes tree RNG again.
 * Invalid used variants/identities/terrain or >4 artillery entries per side fail
 * atomically. An undelivered current type returns UNSUPPORTED. Does not configure
 * full e006 devices, take player control, install contact or dispatch living AI. */
int fist_mission_world_prepare(fist_mission_world *world, const fist_klc_image *height,
                               uint8_t link_mode);

/* Borrow a physical payload, including an orphan, or NULL for invalid/unused
 * slots. The allocation owner remains the authority for current bindings. */
const fist_mission_object *fist_mission_world_object(const fist_mission_world *world,
                                                     uint16_t slot);

/* Consume the canonical physical M1 and publish complete returned shell/muzzle
 * payloads before the next world visit. Shared history belongs to the mission
 * caller; the pool remains the identity/occupancy authority. Returns 0 or -1
 * preserving world/history/output on invalid input. Does not dispatch flight,
 * damage, class updates, input devices or audible requests. */
int fist_mission_world_fire_untargeted(fist_mission_world *world, fist_fire_history *history,
                                       fist_fire_request request, fist_fire_result *out);

/* Complete original ab82 mode selection. Uses the installed platoon descriptor
 * and canonical RNG; phase_random is the caller's already consumed ab03 value.
 * Does not run the parent phase, advance counters, resolve targets or dispatch
 * subsequent commands. Reject a used descriptor selector outside the original
 * UI's four choices; unused retained words do not restrict earlier branches.
 * Returns 0 or -1; every failure preserves the complete world. */
typedef struct {
    uint16_t slot;
    uint16_t phase_random;
} fist_command_selection;

int fist_mission_world_select_command(fist_mission_world *world, fist_command_selection request);

/* Complete ac75 goal assignment for all eight mode entries: first waypoint or
 * automatic formation offset from the physical platoon leader; the other six
 * original entries are genuine returns. Uses fine shared rotation and retained
 * leader heading average. Does not sample headings, advance routes, resolve
 * targets or consume RNG. Invalid used selectors/roster metadata fail without
 * changing any world state. */
int fist_mission_world_assign_command_goal(fist_mission_world *world, uint16_t slot);

/* Complete ab88 eight-entry direction/range/retreat callback, using canonical
 * goals and physical target lifetimes. coarse selects shared numeric precision.
 * Lost targets deliberately resume fresh route/formation navigation; missing
 * goals stay invalid for the navigation throttle stop. Ongoing retreat needs
 * no target. No RNG, route advancement, throttle/gear or parent-bank dispatch.
 * Invalid used fields/payloads fail atomically, preserving the complete world. */
int fist_mission_world_bear_command(fist_mission_world *world, uint16_t slot, bool coarse);

/* Complete ad2f eight-entry throttle and unconditional ad3b profile update.
 * Consume canonical PINF +6 only for a valid leader goal with range >8.
 * Target mode uses retained +99, not navigation range. Publish explicit full
 * drive-control refresh; no RNG, target lookup, route or parent dispatch.
 * Invalid used selectors/state/output preserve the complete world and output. */
int fist_mission_world_throttle_command(fist_mission_world *world, uint16_t slot,
                                        fist_drive_control_events *out);

/* Complete controlled-bank ad3b using the same canonical actor/profile owner.
 * Does not inspect command mode or PINF choices. Atomic failures preserve both. */
int fist_mission_world_update_command_profile(fist_mission_world *world, uint16_t slot,
                                              fist_drive_control_events *out);

/* Complete ad08 route progress. Mode zero consumes the platoon's first waypoint
 * when its goal is valid and the retained unsigned range is <=48; other seven
 * entries are original returns. PINF waypoint mode 3 cycles; 0/1/2 and the
 * original >=4 fallback remove the first point. All headers/unused in-record
 * values survive the original safe-domain copy. At full capacity the newly
 * unused final slot retains its last value, repairing the original neighbor
 * read; cyclic progress never reads past its last active point. No RNG, bearing
 * production, parent dispatch or unrelated world mutation. Invalid used route
 * counts/actor/world metadata fail with the complete world unchanged. */
int fist_mission_world_advance_command_route(fist_mission_world *world, uint16_t slot);

#endif
