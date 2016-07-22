#ifdef __GNUC__
  #define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#else
  #define UNUSED(x) UNUSED_ ## x
#endif

#include "erl_nif.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <appsignal_extension.h>

static ErlNifResourceType *appsignal_transaction_type = NULL;

typedef struct {
  appsignal_transaction_t *transaction;
} transaction_ptr;


static ERL_NIF_TERM
make_ok_tuple(ErlNifEnv *env, ERL_NIF_TERM value)
{
    return enif_make_tuple2(env, enif_make_atom(env, "ok"), value);
}

static ERL_NIF_TERM
make_error_tuple(ErlNifEnv *env, const char *reason)
{
    return enif_make_tuple2(env, enif_make_atom(env, "error"), enif_make_atom(env, reason));
}

static appsignal_string_t StringValueCStr(ErlNifBinary binary)
{
    appsignal_string_t retval;
    retval.len = binary.size;
    retval.buf = (char *)binary.data;
    return retval;
}

static ERL_NIF_TERM _start(ErlNifEnv* env, int UNUSED(arc), const ERL_NIF_TERM UNUSED(argv[]))
{
    appsignal_start();
    return enif_make_atom(env, "ok");
}

static ERL_NIF_TERM _stop(ErlNifEnv* env, int UNUSED(arc), const ERL_NIF_TERM UNUSED(argv[]))
{
    appsignal_stop();
    return enif_make_atom(env, "ok");
}

static ERL_NIF_TERM _start_transaction(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    ErlNifBinary transaction_id, namespace;
    transaction_ptr *ptr;
    ERL_NIF_TERM transaction_ref;

    if (argc != 2) {
      return enif_make_badarg(env);
    }
    if(!enif_inspect_iolist_as_binary(env, argv[0], &transaction_id)) {
        return enif_make_badarg(env);
    }
    if(!enif_inspect_iolist_as_binary(env, argv[1], &namespace)) {
        return enif_make_badarg(env);
    }

    ptr = enif_alloc_resource(appsignal_transaction_type, sizeof(transaction_ptr));
    if(!ptr)
	    return make_error_tuple(env, "no_memory");

    ptr->transaction = appsignal_start_transaction(
        StringValueCStr(transaction_id),
        StringValueCStr(namespace)
    );

    transaction_ref = enif_make_resource(env, ptr);
    enif_release_resource(ptr);

    return make_ok_tuple(env, transaction_ref);
}

static ERL_NIF_TERM _start_event(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    transaction_ptr *ptr;

    if (argc != 1) {
      return enif_make_badarg(env);
    }
    if(!enif_get_resource(env, argv[0], appsignal_transaction_type, (void**) &ptr)) {
	    return enif_make_badarg(env);
    }

    appsignal_start_event(ptr->transaction);

    return enif_make_atom(env, "ok");
}

static ERL_NIF_TERM _finish_event(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    transaction_ptr *ptr;
    ErlNifBinary name, title, body;
    int bodyFormat;

    if (argc != 5) {
      return enif_make_badarg(env);
    }
    if(!enif_get_resource(env, argv[0], appsignal_transaction_type, (void**) &ptr)) {
	    return enif_make_badarg(env);
    }
    if(!enif_inspect_iolist_as_binary(env, argv[1], &name)) {
        return enif_make_badarg(env);
    }
    if(!enif_inspect_iolist_as_binary(env, argv[2], &title)) {
        return enif_make_badarg(env);
    }
    if(!enif_inspect_iolist_as_binary(env, argv[3], &body)) {
        return enif_make_badarg(env);
    }
    if(!enif_get_int(env, argv[4], &bodyFormat)) {
        return enif_make_badarg(env);
    }

    appsignal_finish_event(
        ptr->transaction,
        StringValueCStr(name),
        StringValueCStr(title),
        StringValueCStr(body),
        bodyFormat
    );

    return enif_make_atom(env, "ok");
}

static ERL_NIF_TERM _set_error(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    transaction_ptr *ptr;
    ErlNifBinary error, message, backtrace;

    if (argc != 4) {
      return enif_make_badarg(env);
    }
    if(!enif_get_resource(env, argv[0], appsignal_transaction_type, (void**) &ptr)) {
	    return enif_make_badarg(env);
    }
    if(!enif_inspect_iolist_as_binary(env, argv[1], &error)) {
        return enif_make_badarg(env);
    }
    if(!enif_inspect_iolist_as_binary(env, argv[2], &message)) {
        return enif_make_badarg(env);
    }
    if(!enif_inspect_iolist_as_binary(env, argv[3], &backtrace)) {
        return enif_make_badarg(env);
    }

    appsignal_set_transaction_error(
        ptr->transaction,
        StringValueCStr(error),
        StringValueCStr(message),
        StringValueCStr(backtrace)
    );

    return enif_make_atom(env, "ok");
}

static ERL_NIF_TERM _set_sample_data(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    transaction_ptr *ptr;
    ErlNifBinary key, payload;

    if (argc != 3) {
      return enif_make_badarg(env);
    }
    if(!enif_get_resource(env, argv[0], appsignal_transaction_type, (void**) &ptr)) {
	    return enif_make_badarg(env);
    }
    if(!enif_inspect_iolist_as_binary(env, argv[1], &key)) {
        return enif_make_badarg(env);
    }
    if(!enif_inspect_iolist_as_binary(env, argv[2], &payload)) {
        return enif_make_badarg(env);
    }

    appsignal_set_transaction_sample_data(
        ptr->transaction,
        StringValueCStr(key),
        StringValueCStr(payload)
    );

    return enif_make_atom(env, "ok");
}

static ERL_NIF_TERM _set_action(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    transaction_ptr *ptr;
    ErlNifBinary action;

    if (argc != 2) {
      return enif_make_badarg(env);
    }
    if(!enif_get_resource(env, argv[0], appsignal_transaction_type, (void**) &ptr)) {
	    return enif_make_badarg(env);
    }
    if(!enif_inspect_iolist_as_binary(env, argv[1], &action)) {
        return enif_make_badarg(env);
    }

    appsignal_set_transaction_action(
        ptr->transaction,
        StringValueCStr(action)
    );

    return enif_make_atom(env, "ok");
}

static ERL_NIF_TERM _set_queue_start(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    transaction_ptr *ptr;
    long start;

    if (argc != 2) {
      return enif_make_badarg(env);
    }
    if(!enif_get_resource(env, argv[0], appsignal_transaction_type, (void**) &ptr)) {
	    return enif_make_badarg(env);
    }
    if(!enif_get_long(env, argv[1], &start)) {
        return enif_make_badarg(env);
    }

    appsignal_set_transaction_queue_start(
        ptr->transaction,
        start
    );

    return enif_make_atom(env, "ok");
}

static ERL_NIF_TERM _set_meta_data(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    transaction_ptr *ptr;
    ErlNifBinary key, value;

    if (argc != 3) {
      return enif_make_badarg(env);
    }
    if(!enif_get_resource(env, argv[0], appsignal_transaction_type, (void**) &ptr)) {
	    return enif_make_badarg(env);
    }
    if(!enif_inspect_iolist_as_binary(env, argv[1], &key)) {
        return enif_make_badarg(env);
    }
    if(!enif_inspect_iolist_as_binary(env, argv[1], &value)) {
        return enif_make_badarg(env);
    }

    appsignal_set_transaction_metadata(
        ptr->transaction,
        StringValueCStr(key),
        StringValueCStr(value)
    );

    return enif_make_atom(env, "ok");
}

static ERL_NIF_TERM _finish(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    transaction_ptr *ptr;
    int sample;

    if (argc != 1) {
      return enif_make_badarg(env);
    }
    if(!enif_get_resource(env, argv[0], appsignal_transaction_type, (void**) &ptr)) {
	    return enif_make_badarg(env);
    }

    sample = appsignal_finish_transaction(ptr->transaction);

    if (sample == 1) {
      return enif_make_atom(env, "sample");
    } else {
      return enif_make_atom(env, "no_sample");
    }
}

static ERL_NIF_TERM _complete(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    transaction_ptr *ptr;

    if (argc != 1) {
      return enif_make_badarg(env);
    }
    if(!enif_get_resource(env, argv[0], appsignal_transaction_type, (void**) &ptr)) {
	    return enif_make_badarg(env);
    }

    appsignal_complete_transaction(ptr->transaction);

    return enif_make_atom(env, "ok");
}

static void destruct_appsignal_transaction(ErlNifEnv *UNUSED(env), void *arg) {
  transaction_ptr *ptr = (transaction_ptr *)arg;
  appsignal_free_transaction(ptr->transaction);
}

static ERL_NIF_TERM _set_gauge(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    ErlNifBinary key;
    double value;

    if (argc != 2) {
      return enif_make_badarg(env);
    }
    if(!enif_inspect_iolist_as_binary(env, argv[0], &key)) {
        return enif_make_badarg(env);
    }
    if(!enif_get_double(env, argv[1], &value)) {
        return enif_make_badarg(env);
    }

    appsignal_set_gauge(StringValueCStr(key), value);

    return enif_make_atom(env, "ok");
}

static ERL_NIF_TERM _increment_counter(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    ErlNifBinary key;
    int count;

    if (argc != 2) {
      return enif_make_badarg(env);
    }
    if(!enif_inspect_iolist_as_binary(env, argv[0], &key)) {
        return enif_make_badarg(env);
    }
    if(!enif_get_int(env, argv[1], &count)) {
        return enif_make_badarg(env);
    }

    appsignal_increment_counter(StringValueCStr(key), count);

    return enif_make_atom(env, "ok");
}

static ERL_NIF_TERM _add_distribution_value(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    ErlNifBinary key;
    double value;

    if (argc != 2) {
      return enif_make_badarg(env);
    }
    if(!enif_inspect_iolist_as_binary(env, argv[0], &key)) {
        return enif_make_badarg(env);
    }
    if(!enif_get_double(env, argv[1], &value)) {
        return enif_make_badarg(env);
    }

    appsignal_add_distribution_value(StringValueCStr(key), value);

    return enif_make_atom(env, "ok");
}

static int on_load(ErlNifEnv* env, void** UNUSED(priv), ERL_NIF_TERM UNUSED(info))
{
    ErlNifResourceType *rt;

    rt = enif_open_resource_type(
        env,
        "appsignal_nif",
        "appsignal_transaction_type",
				destruct_appsignal_transaction,
        ERL_NIF_RT_CREATE,
        NULL
    );
    if (!rt) {
	    return -1;
    }
    appsignal_transaction_type = rt;

    return 0;
}

static int on_reload(ErlNifEnv* UNUSED(env), void** UNUSED(priv_data), ERL_NIF_TERM UNUSED(load_info))
{
    return 0;
}

static int on_upgrade(ErlNifEnv* UNUSED(env), void** UNUSED(priv), void** UNUSED(old_priv_data), ERL_NIF_TERM UNUSED(load_info))
{
    return 0;
}

static ErlNifFunc nif_funcs[] =
{
    {"_start", 0, _start, 0},
    {"_stop", 0, _stop, 0},
    {"_start_transaction", 2, _start_transaction, 0},
    {"_start_event", 1, _start_event, 0},
    {"_finish_event", 5, _finish_event, 0},
    {"_set_error", 4, _set_error, 0},
    {"_set_sample_data", 3, _set_sample_data, 0},
    {"_set_action", 2, _set_action, 0},
    {"_set_queue_start", 2, _set_queue_start, 0},
    {"_set_meta_data", 3, _set_meta_data, 0},
    {"_finish", 1, _finish, 0},
    {"_complete", 1, _complete, 0},
    {"_set_gauge", 2, _set_gauge, 0},
    {"_increment_counter", 2, _increment_counter, 0},
    {"_add_distribution_value", 2, _add_distribution_value, 0}
};

ERL_NIF_INIT(Elixir.Appsignal.Nif, nif_funcs, on_load, on_reload, on_upgrade, NULL)
