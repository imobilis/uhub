/*
 * uhub - A tiny ADC p2p connection hub
 * Copyright (C) 2007-2009, Jan Vidar Krey
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef HAVE_UHUB_HUB_H
#define HAVE_UHUB_HUB_H

enum status_message
{
	status_msg_hub_full                  = -1,  /* hub is full */
	status_msg_hub_disabled              = -2,  /* hub is disabled */
	status_msg_hub_registered_users_only = -3,  /* hub is for registered users only */
	status_msg_inf_error_nick_missing    = -4,  /* no nickname given */
	status_msg_inf_error_nick_multiple   = -5,  /* multiple nicknames given */
	status_msg_inf_error_nick_invalid    = -6,  /* generic/unkown */
	status_msg_inf_error_nick_long       = -7,  /* nickname too long */
	status_msg_inf_error_nick_short      = -8,  /* nickname too short */
	status_msg_inf_error_nick_spaces     = -9,  /* nickname cannot start with spaces */
	status_msg_inf_error_nick_bad_chars  = -10, /* nickname contains chars below ascii 32 */
	status_msg_inf_error_nick_not_utf8   = -11, /* nickname is not valid utf8 */
	status_msg_inf_error_nick_taken      = -12, /* nickname is in use */
	status_msg_inf_error_nick_restricted = -13, /* nickname cannot be used on this hub */
	status_msg_inf_error_cid_invalid     = -14, /* CID is not valid (generic error) */
	status_msg_inf_error_cid_missing     = -15, /* CID is not specified */
	status_msg_inf_error_cid_taken       = -16, /* CID is taken (already logged in?). */
	status_msg_inf_error_pid_missing     = -17, /* PID is not specified */
	status_msg_inf_error_pid_invalid     = -18, /* PID is invalid */
	status_msg_ban_permanently           = -19, /* Banned permanently */
	status_msg_ban_temporarily           = -20, /* Banned temporarily */
	status_msg_auth_invalid_password     = -21, /* Password is wrong */
	status_msg_auth_user_not_found       = -22, /* User not found in password database */
	status_msg_error_no_memory           = -23, /* Hub is out of memory */
	
	status_msg_user_share_size_low       = -40, /* User is not sharing enough. */
	status_msg_user_share_size_high      = -41, /* User is sharing too much. */
	status_msg_user_slots_low            = -42, /* User has too few slots open. */
	status_msg_user_slots_high           = -43, /* User has too many slots open. */
	status_msg_user_hub_limit_low        = -44, /* Use is on too few hubs. */
	status_msg_user_hub_limit_high       = -45, /* Use is on too many hubs. */

};


enum hub_state
{
	hub_status_uninitialized = 0, /**<<<"Hub is uninitialized" */
	hub_status_running       = 1, /**<<<"Hub is running (normal operation)" */
	hub_status_restart       = 2, /**<<<"Hub is restarting (re-reading configuration, etc)" */
	hub_status_shutdown      = 3, /**<<<"Hub is shutting down, but not yet stopped. */
	hub_status_stopped       = 4, /**<<<"Hub is stopped (Pretty much the same as initialized) */
	hub_status_disabled      = 5, /**<<<"Hub is disabled (Running, but not accepting users) */
};

/**
 * Always updated each minute.
 */
struct hub_stats
{
	size_t net_tx;
	size_t net_rx;
	size_t net_tx_peak;
	size_t net_rx_peak;
	size_t net_tx_total;
	size_t net_rx_total;
};

struct hub_info
{
	int fd_tcp;
#ifdef ADC_UDP_OPERATION
	int fd_udp;
#endif
	struct event ev_accept;
	struct event ev_timer;
#ifdef ADC_UDP_OPERATION
	struct event ev_datagram;
#endif
	struct hub_stats stats;
	struct event_queue* queue;
	struct event_base* evbase;
	struct hub_config* config;
	struct user_manager* users;
	struct acl_handle* acl;
	struct adc_message* command_info;    /* The hub's INF command */
	struct adc_message* command_support; /* The hub's SUP command */
	struct adc_message* command_motd;    /* The message of the day */
	struct adc_message* command_banner;  /* The default welcome message */
	time_t tm_started;
	int status;
#ifdef SSL_SUPPORT
	SSL_METHOD* ssl_method;
	SSL_CTX* ssl_ctx;
#endif /*  SSL_SUPPORT */
};

/**
 * This is the message pre-routing centre.
 *
 * Any message coming in to the hub comes through here first,
 * and will be routed further if valid.
 *
 * @return 0 on success, -1 on error
 */
extern int hub_handle_message(struct user* u, const char* message, size_t length);

/**
 * Handle protocol support/subscription messages received clients.
 *
 * @return 0 on success, -1 on error
 */
extern int hub_handle_support(struct user* u, struct adc_message* cmd);

/**
 * Handle password messages received from clients.
 *
 * @return 0 on success, -1 on error
 */
extern int hub_handle_password(struct user* u, struct adc_message* cmd);

/**
 * Handle chat messages received from clients.
 * @return 0 on success, -1 on error.
 */
extern int hub_handle_chat_message(struct user* u, struct adc_message* cmd);

/**
 * Used internally by hub_handle_info
 * @return 1 if nickname is OK, or 0 if nickname is not accepted.
 */
extern int  hub_handle_info_check_nick(struct user* u, struct adc_message* cmd);

/**
 * Used internally by hub_handle_info
 * @return 1 if CID/PID is OK, or 0 if not valid.
 */
extern int  hub_handle_info_check_cid(struct user* u, struct adc_message* cmd);

/**
 * Can only be used by administrators or operators.
 *
 * @return 0 on success, -1 on error
 */
extern int hub_handle_kick(struct user* u, struct adc_message* cmd);

#ifdef ADC_UDP_OPERATION
/**
 * Handle incoming autocheck message.
 */
extern int hub_handle_autocheck(struct user* u, struct adc_message* cmd);
#endif

/**
 * Send the support line for the hub to a particular user.
 * Only used during the initial handshake.
 */
extern void hub_send_support(struct user* u);

/**
 * Send a message assigning a SID for a user.
 * This is only sent after hub_send_support() during initial handshake.
 */
extern void hub_send_sid(struct user* u);

/**
 * Send a 'ping' message to user.
 */
extern void hub_send_ping(struct user* user);

/**
 * Send a message containing hub information to a particular user.
 * This is sent during user connection, but can safely be sent at any
 * point later.
 */
extern void hub_send_hubinfo(struct user* u);

/**
 * Send handshake. This basically calls
 * hub_send_support() and hub_send_sid()
 */
extern void hub_send_handshake(struct user* u);

/**
 * Send a welcome message containing the message of the day to 
 * one particular user. This can be sent in any point in time.
 */
extern void hub_send_motd(struct user* u);

/**
 * Send a password challenge to a user.
 * This is only used if the user tries to access the hub using a
 * password protected nick name.
 */
extern void hub_send_password_challenge(struct user* u);

/**
 * Send an autocheck message to a user.
 * This is basically a UDP message. The user's client can then determine
 * if UDP communication works by either hole punching or configuring UPnP.
 */
extern void hub_send_autocheck(struct user* u, uint16_t port, const char* token);

/**
 * This starts the hub.
 */
extern struct hub_info* hub_start_service(struct hub_config* config);

/**
 * This shuts down the hub.
 */
extern void hub_shutdown_service(struct hub_info* hub);

/**
 * This configures the hub.
 */
extern void hub_set_variables(struct hub_info* hub, struct acl_handle* acl);

/**
 * This frees the configuration of the hub.
 */
extern void hub_free_variables(struct hub_info* hub);

/**
 * Returns a string for the given status_message (See enum status_message).
 */
extern const char* hub_get_status_message(struct hub_info* hub, enum status_message msg);
extern const char* hub_get_status_message_log(struct hub_info* hub, enum status_message msg);


/**
 * Sends a status_message to a user.
 */
extern void hub_send_status(struct user* user, enum status_message msg, enum msg_status_level level);

/**
 * Returns the number of logged in users on the hub.
 */
extern size_t hub_get_user_count(struct hub_info* hub);

/**
 * Returns the maximum number of allowed users on the hub.
 */
extern size_t hub_get_max_user_count(struct hub_info* hub);

/**
 * Returns the accumulated shared size for all logged in
 * users on the hub.
 */
extern uint64_t hub_get_shared_size(struct hub_info* hub);

/**
 * Returns the accumulated number of files for all logged
 * in users on the hub.
 */
extern uint64_t hub_get_shared_files(struct hub_info* hub);

/**
 * Returns the minimal share size limit as enforced by
 * this hub's configuration.
 */
extern uint64_t hub_get_min_share(struct hub_info* hub);

/**
 * Returns the minimal share size limit as enforced by
 * this hub's configuration.
 */
extern uint64_t hub_get_max_share(struct hub_info* hub);

/**
 * Returns the minimum upload slot limit as enforced by
 * this hub's configuration.
 * Users with fewer slots in total will not be allowed
 * to enter the hub.
 * @return limit or 0 if no limit.
 */
extern size_t hub_get_min_slots(struct hub_info* hub);

/**
 * Returns the maximum upload slot limit as enforced by
 * this hub's configuration.
 * Users with more allowed upload slots will not be
 * allowed to enter the hub.
 * @return limit or 0 if no limit.
 */
extern size_t hub_get_max_slots(struct hub_info* hub);

/**
 * Returns the maximum number of hubs a user can
 * be logged in to simultaneously as a regular user (guest).
 * Users on more hubs will not be allowed to stay on this hub.
 * @return limit or 0 if no limit.
 */
extern size_t hub_get_max_hubs_user(struct hub_info* hub);
extern size_t hub_get_min_hubs_user(struct hub_info* hub);

/**
 * Returns the maximum number of hubs a user can
 * be logged in to simultaneously as a registered user (password required).
 * Users on more hubs will not be allowed to stay on this hub.
 * @return limit or 0 if no limit.
 */
extern size_t hub_get_max_hubs_reg(struct hub_info* hub);
extern size_t hub_get_min_hubs_reg(struct hub_info* hub);

/**
 * Returns the maximum number of hubs a user can
 * be logged in to simultaneously as an operator.
 * Users who are operator on more than this amount of hubs
 * will not be allowed to stay on this hub.
 * @return limit or 0 if no limit.
 */
extern size_t hub_get_max_hubs_op(struct hub_info* hub);
extern size_t hub_get_min_hubs_op(struct hub_info* hub);

/**
 * Returns the maximum number of hubs a user can
 * be logged in to simultaneously regardless of the type of user.
 */
extern size_t hub_get_max_hubs_total(struct hub_info* hub);

/**
 * Schedule runslice.
 */
extern void hub_schedule_runslice(struct hub_info* hub);

/**
 * Run event loop.
 */
extern void hub_event_loop(struct hub_info* hub);


#endif /* HAVE_UHUB_HUB_H */

