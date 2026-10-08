#ifndef FIST_SIM_MISSION_WORLD_H
#define FIST_SIM_MISSION_WORLD_H

#include "assets/klc.h"
#include "assets/orders.h"
#include "assets/units.h"
#include "sim/automatic_fire.h"
#include "sim/ground_support.h"
#include "sim/object_pool.h"
#include "sim/other_damage.h"
#include "sim/primary_fire.h"
#include "sim/projectile_flight.h"
#include "sim/projectile_launch.h"
#include "sim/random.h"
#include "sim/smoke.h"
#include "sim/tree.h"
#include "sim/vehicle_damage.h"
#include "sim/vehicle_motion.h"
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
 * Primary launch installs dynamic shell 8 and muzzle 18. Canonical combat
 * visits publish explosion 4 and consume delivered death/effect/retirement
 * classes. Saved restoration of shell/explosion classes and living dispatch
 * remain separate. */
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
    fist_surface_air_missile surface_air;
    fist_support_marker support_marker;
} fist_mission_object;

typedef struct {
    uint16_t trees;
    uint16_t counted_static;
    uint16_t artillery_count[FIST_DAMAGE_SIDES];
    /* Actual runtime identities, in original registry order; no saved near
     * pointer translation. After preparation, entries beyond each count are
     * NO_SLOT. */
    fist_artillery_resource artillery[FIST_DAMAGE_SIDES][FIST_MISSION_ARTILLERY_SIDE_SLOTS];
    uint8_t prepared;
} fist_mission_preparation;

enum {
    FIST_NOTICE_TARGET = 0,
    FIST_NOTICE_SAM_EMPTY = 1,
    FIST_NOTICE_SAM_TARGET = 2,
    FIST_NOTICE_SAM_CAPACITY = 3,
    FIST_NOTICE_SMOKE_EMPTY = 4,
    FIST_NOTICE_AIR_CONFIRMED = 5,
    FIST_NOTICE_AIR_UNAVAILABLE = 6,
    FIST_NOTICE_ARTILLERY_CONFIRMED = 7,
    FIST_NOTICE_ARTILLERY_UNAVAILABLE = 8
};

/* Single original 969e/96a0 display owner. Target class/side/variant or typed
 * missile failure, never a guest text address. duration is shared. */
typedef struct {
    uint16_t duration;
    uint16_t type;
    uint8_t variant;
    bool enemy;
    uint8_t kind;
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
    /* Original c047 selector history 9fdd, separate from bf3c's cooldown. */
    uint16_t sound_selector;
    fist_mission_support support;
    fist_timed_advisory advisory;
    /* Original fixed artillery text at 7a52 with duration 7a50 and refresh 87c2.
     * Typed message presence, without storing a guest text buffer. */
    uint16_t artillery_message_ticks;
} fist_mission_world;

/* Complete original support reset/configuration. Resets queue admission and
 * side cooldowns to clock-480; retains global request age and queue tails.
 * Canonical artillery resource counts belong to mission preparation. Null or
 * invalid input preserves world; configuration may alias its existing owner. */
int fist_mission_world_configure_support(fist_mission_world *world,
                                         const fist_support_configuration *configuration,
                                         uint16_t clock);

/* Complete ae5c station child using canonical target lifetimes, original
 * literal preferences and the existing mechanical/voice owners. Invalid used
 * variants fail atomically; released targets cannot bind successors. No RNG,
 * parent dispatch or PCM. All failures preserve world/output. */
int fist_mission_world_select_station(fist_mission_world *world,
                                      fist_ground_station_request request,
                                      fist_ground_station_result *out);

/* Complete b0be child and queued support request producers. Explicit config is
 * required only on admitted support paths. Typed clocks repair the third/fourth
 * gun defect; retained references prevent successor binding. Smoke attempts use
 * the proved source-unmatched support branch independently of emitted sound or
 * device/bank responses. Dispatch/flight/PCM remain separate; invalid used
 * inputs preserve world/output atomically. */
int fist_mission_world_request_support(fist_mission_world *world, const fist_klc_image *height,
                                       fist_ground_support_request request,
                                       fist_ground_support_result *out);

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
 * Return 0, or -1 for invalid state/input, including a missing/mistagged
 * projection; all failures preserve world/output, including notification
 * history. */
int fist_mission_world_discover_targets(fist_mission_world *world, const fist_klc_image *height,
                                        fist_target_discovery_request request,
                                        fist_target_discovery_result *out);

/* Actual shared e21c class/variant aim offsets. Inputs are canonical physical
 * projections; outputs own their poses. No visibility, references or mutation.
 */
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
 * Automatic admission sets bit 128 even after a failed attempt. Stale
 * references cannot address successors. Selected live type-26 modes outside the
 * authored four-message domain fail atomically rather than read adjacent text.
 * Unused behavior/height/variant inputs do not restrict original early
 * branches. All failures preserve complete world/output; no parent, aiming or
 * PCM. */
int fist_mission_world_acquire_target(fist_mission_world *world, const fist_klc_image *height,
                                      fist_target_acquisition_request request,
                                      fist_target_acquisition_result *out);

/* Complete f69:abd5/a18e target geometry. Produce +9b heading, existing +38
 * turret elevation and packed +99 range; retain all on null/lost target except
 * invalidating its runtime reference. No RNG, throttle or parent dispatch.
 * Invalid used projection/state fails atomically. */
int fist_mission_world_aim_target(fist_mission_world *world, uint16_t slot, bool coarse);

/* Complete class motion followed by target-aware turret slew. M1/M3 retain
 * aim feedback; T80/BMP refresh it after translation, using captured live
 * targets. Lost references retain feedback and cannot bind a reused slot.
 * No command, phase, terrain, RNG, devices or orders are consumed. Invalid
 * used state/projection preserves the complete world and event output. */
int fist_mission_world_ground_motion(fist_mission_world *world, uint16_t slot, bool coarse,
                                     fist_vehicle_motion_events *out);

void fist_mission_world_reset(fist_mission_world *world);

/* Complete normal-side d84a/43c1 saved-object installation for delivered
 * classes, using decoded immutable definitions. Initialize only participating
 * ground actors, in input order; retain final RNG and all physical orphans.
 * Input units/random are borrowed; no input views survive. Units stay
 * unchanged. Random stays unchanged unless it aliases out->random; that
 * supported reload replaces the old world using its current RNG as the new
 * initial state. Returns OK, UNAVAILABLE for physical exhaustion, UNSUPPORTED
 * for an undelivered type, or -1 for invalid data/allocation. Every failure
 * preserves out. This owns neither terrain/contact installation nor living
 * class dispatch/devices. */
int fist_mission_world_initialize(const fist_units *units, const fist_random *random,
                                  uint8_t link_mode, fist_mission_world *out);

/* Original mission-start census/list zeros followed by complete d755 dispatch
 * for every delivered current binding. Visit registry order, including deleted
 * nonparticipants; preserve physical orphans. Reset ground fields, initialize
 * tree/target/artillery preparation, sample actual op-54 heights and release
 * temporary objects. Height is borrowed; no views survive. Reuses class
 * defaults, canonical RNG and pool release. Repeated preparation consumes tree
 * RNG again. Invalid used variants/identities/terrain or >4 artillery entries
 * per side fail atomically. An undelivered current type returns UNSUPPORTED.
 * Does not configure full e006 devices, take player control, install contact or
 * dispatch living AI. */
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
 * Invalid used fields/payloads fail atomically, preserving the complete world.
 */
int fist_mission_world_bear_command(fist_mission_world *world, uint16_t slot, bool coarse);

/* Complete ad2f eight-entry throttle and unconditional ad3b profile update.
 * Consume canonical PINF +6 only for a valid leader goal with range >8.
 * Target mode uses retained +99, not navigation range. Publish explicit full
 * drive-control refresh; no RNG, target lookup, route or parent dispatch.
 * Invalid used selectors/state/output preserve the complete world and output.
 */
int fist_mission_world_throttle_command(fist_mission_world *world, uint16_t slot,
                                        fist_drive_control_events *out);

/* Complete controlled-bank ad3b using the same canonical actor/profile owner.
 * Does not inspect command mode or PINF choices. Atomic failures preserve both.
 */
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

/* Complete b053/f69:b329 physical roster promotion. A member-zero actor uses
 * no platoon/roster input. Null/wreck predecessors bypass actor motion flags;
 * other ground/retiring predecessors use actual +19 bits. Mutate the sole
 * canonical roster and existing typed member bytes, clearing goal validity.
 * No orders, RNG, target lookup, census, player selection or parent dispatch.
 * Invalid used member/index/identity/payload fails before any world mutation.
 */
int fist_mission_world_promote_member(fist_mission_world *world, uint16_t slot);

/* Complete ae66/af1c obstacle state/countdown and ordered fifteen-direction
 * search. Use current physical registry bodies and shared 24-sample prediction;
 * retain original wrapped counts and the distinct selected turn. No parent
 * dispatch, orders, RNG, target resolution or motion integration. Used invalid
 * actor/body metadata fails with the entire world unchanged. */
int fist_mission_world_maneuver(fist_mission_world *world, uint16_t slot, bool coarse);

/* Complete reaching b059/f69:b2a0: a physically identified candidate supplies
 * the angular gate and 24-step prediction. Any nonzero maneuver selector uses
 * hull rotation; zero uses retained velocity. A hit ORs control bit 8. No
 * target lookup/filter, registry scan or other state mutation; failures are
 * atomic. */
typedef struct {
    uint16_t slot;
    uint16_t candidate;
    bool coarse;
} fist_obstacle_observation;

int fist_mission_world_observe_obstacle(fist_mission_world *world,
                                        fist_obstacle_observation request);

/* Complete b017: control bit 4 or retained target presence returns without
 * touching target payloads/lifetimes/RNG. Otherwise consume one canonical draw
 * and only the proved conditional second draw to update requested turret
 * offset. Existing owners clear targets. Used invalid state fails atomically.
 */
int fist_mission_world_idle_turret(fist_mission_world *world, uint16_t slot);

/* Complete af97/afa2, including genuine missile readiness/loading/constructor,
 * selected display and c047 logical request. Use canonical orders and exact
 * live retained targets only when their payload is used. No parent dispatch,
 * phase/RNG advancement, missile flight or PCM playback. Invalid used state
 * preserves the complete world and output; capacity retains reserve
 * consumption. */
int fist_mission_world_automatic_fire(fist_mission_world *world,
                                      fist_automatic_fire_request request,
                                      fist_automatic_fire_result *out);

/* Complete M3/BMP 8711/96c0 without the parent's admission gates. Arming/busy
 * branches do not use target payloads/orders/RNG; empty reserve fails through
 * the actual display path. Used invalid target/allocation state is atomic. */
int fist_mission_world_ready_surface_air(fist_mission_world *world,
                                         fist_surface_air_request request,
                                         fist_automatic_fire_result *out);

/* Original command-caller values, distinct from normal player/UI selection.
 * Diagnostic selection captures a physical lifetime and never binds a reused
 * successor. diagnostic_saved retains the original displayed97ee word; its
 * producer/device configuration is not inferred from a zero-filled image. */
typedef struct {
    uint16_t slot;
    uint16_t tick;
    uint16_t clock;
    uint16_t voice_gate;
    uint16_t sound_source;
    uint16_t diagnostic_saved;
    fist_object_reference diagnostic_actor;
    uint8_t inhibition;
    uint8_t notice_context;
    uint8_t link_mode;
    bool coarse;
} fist_ground_phase_request;

/* Complete semantic b152 observation. Portable presentation uses the platoon
 * identity in place of a guest descriptor address and a captured candidate
 * identity in place of a near word. It owns every value and borrows no state.
 */
typedef struct {
    fist_object_reference actor;
    fist_object_reference candidate;
    uint16_t throttle_bits;
    uint16_t platoon_speed;
    uint16_t route_points;
    uint16_t saved;
    uint16_t navigation_range;
    uint8_t mode;
    uint8_t discovery_count;
    uint8_t maneuver;
    uint8_t platoon;
    uint8_t member;
    bool inhibited;
    bool candidate_live;
} fist_ground_diagnostic;

enum { FIST_GROUND_COMMAND_CALLBACKS = 16, FIST_GROUND_COMMAND_NO_CALLBACK = UINT8_MAX };

typedef struct {
    fist_drive_control_events drive;
    fist_target_discovery_result discovery;
    fist_target_acquisition_result acquisition;
    fist_automatic_fire_result fire;
    fist_ground_station_result station;
    fist_ground_support_result support;
    fist_ground_diagnostic diagnostic;
    uint16_t phase_random;
    uint8_t callback;
    bool automatic;
    bool heading_sampled;
    bool diagnostic_emitted;
} fist_ground_phase_result;

/* Complete canonical ab03: draw before inhibition, wrap the existing +42
 * counter, sample signed heading history, dispatch both complete original
 * sixteen-entry banks through their existing owners, then produce selected
 * semantic diagnostics. Genuine controlled b111 entries are explicit returns.
 * Height/configuration/caller fields are required only when their path uses
 * them. Errors preserve the entire world/output, including the initial draw.
 * This does not advance class phase, configure devices or complete living
 * class/battle/PCM dispatch. */
int fist_mission_world_ground_command(fist_mission_world *world, const fist_klc_image *height,
                                      fist_ground_phase_request request,
                                      fist_ground_phase_result *out);

#endif
