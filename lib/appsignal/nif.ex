defmodule Appsignal.Nif do
  @moduledoc false

  @on_load :init

  def init do
    path = :filename.join(:code.priv_dir(:appsignal), 'appsignal_extension')
    :ok = :erlang.load_nif(path, 1)
  end

  def start do
    _start
  end

  def stop do
    _stop
  end

  def start_transaction(transaction_id, namespace) do
    _start_transaction(transaction_id, namespace)
  end

  def start_event(transaction_resource) do
    _start_event(transaction_resource)
  end

  def finish_event(transaction_resource, name, title, body, body_format) do
    _finish_event(transaction_resource, name, title, body, body_format)
  end

  def set_error(transaction_resource, error, message, backtrace) do
    _set_error(transaction_resource, error, message, backtrace)
  end

  def set_sample_data(transaction_resource, key, payload) do
    _set_sample_data(transaction_resource, key, payload)
  end

  def set_action(transaction_resource, action) do
    _set_action(transaction_resource, action)
  end

  def set_queue_start(transaction_resource, start) do
    _set_queue_start(transaction_resource, start)
  end

  def set_meta_data(transaction_resource, key, value) do
    _set_meta_data(transaction_resource, key, value)
  end

  def finish(transaction_resource) do
    _finish(transaction_resource)
  end

  def complete(transaction_resource) do
    _complete(transaction_resource)
  end

  def set_gauge(key, value) do
    _set_gauge(key, value)
  end

  def increment_counter(key, count) do
    _increment_counter(key, count)
  end

  def add_distribution_value(key, value) do
    _add_distribution_value(key, value)
  end

  def _start do
    exit(:nif_library_not_loaded)
  end

  def _stop do
    exit(:nif_library_not_loaded)
  end

  def _start_transaction(_id, _namespace) do
    exit(:nif_library_not_loaded)
  end

  def _start_event(_transaction_resource) do
    exit(:nif_library_not_loaded)
  end

  def _finish_event(_transaction_resource, _name, _title, _body, _body_format) do
    exit(:nif_library_not_loaded)
  end

  def _set_error(_transaction_resource, _error, _message, _backtrace) do
    exit(:nif_library_not_loaded)
  end

  def _set_sample_data(_transaction_resource, _key, _payload) do
    exit(:nif_library_not_loaded)
  end

  def _set_action(_transaction_resource, _action) do
    exit(:nif_library_not_loaded)
  end

  def _set_queue_start(_transaction_resource, _start) do
    exit(:nif_library_not_loaded)
  end

  def _set_meta_data(_transaction_resource, _key, _value) do
    exit(:nif_library_not_loaded)
  end

  def _finish(_transaction_resource) do
    exit(:nif_library_not_loaded)
  end

  def _complete(_transaction_resource) do
    exit(:nif_library_not_loaded)
  end

  def _set_gauge(_key, _value) do
    exit(:nif_library_not_loaded)
  end

  def _increment_counter(_key, _count) do
    exit(:nif_library_not_loaded)
  end

  def _add_distribution_value(_key, _value) do
    exit(:nif_library_not_loaded)
  end
end
