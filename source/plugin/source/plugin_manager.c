/*
 *	Plugin Library by Parra Studios
 *	A library for plugins at run-time into a process.
 *
 *	Copyright (C) 2016 - 2022 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 *
 */

/* -- Headers -- */

#include <plugin/plugin_manager.h>

/* -- Declarations -- */

struct plugin_manager_iterate_cb_type
{
	plugin_manager manager;
	int (*iterator)(plugin_manager, plugin, void *);
	void *data;
};

/* -- Private Methods -- */

static int plugin_manager_iterate_cb(set s, set_key key, set_value val, set_cb_iterate_args args);
static int plugin_manager_destroy_cb(set s, set_key key, set_value val, set_cb_iterate_args args);

/* -- Methods -- */

int plugin_manager_initialize(plugin_manager manager, const char *name, const char *environment_library_path, const char *default_library_path, plugin_manager_interface iface, void *impl)
{
	manager->name = name;
	manager->iface = iface;
	manager->impl = impl;

	if (manager->plugins == NULL)
	{
		manager->plugins = set_create(&hash_callback_str, &comparable_callback_str);

		if (manager->plugins == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid plugin manager set initialization");

			plugin_manager_destroy(manager);

			return 1;
		}
	}

	if (manager->library_path == NULL)
	{
		manager->library_path = environment_variable_path_create(environment_library_path, default_library_path);

		if (manager->library_path == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid plugin manager library path initialization");

			plugin_manager_destroy(manager);

			return 1;
		}
	}

	return 0;
}

const char *plugin_manager_type(plugin_manager manager)
{
	return manager->type;
}

char *plugin_manager_library_path(plugin_manager manager)
{
	return manager->library_path;
}

void *plugin_manager_impl(plugin_manager manager)
{
	return manager->impl;
}

int plugin_manager_register(plugin_manager manager, plugin p)
{
	const char *name = plugin_name(p);

	if (set_get(manager->plugins, (set_key)name) != NULL)
	{
		return 1;
	}

	return set_insert(manager->plugins, (set_key)name, p);
}

plugin plugin_manager_get(plugin_manager manager, const char *name)
{
	return set_get(manager->plugins, (set_key)name);
}

int plugin_manager_iterate_cb(set s, set_key key, set_value val, set_cb_iterate_args args)
{
	(void)s;
	(void)key;

	if (val != NULL && args != NULL)
	{
		struct plugin_manager_iterate_cb_type *args_ptr = (struct plugin_manager_iterate_cb_type *)args;
		return args_ptr->iterator(args_ptr->manager, (plugin)val, args_ptr->data);
	}

	return 0;
}

void plugin_manager_iterate(plugin_manager manager, int (*iterator)(plugin_manager, plugin, void *), void *data)
{
	if (iterator == NULL)
	{
		return;
	}

	struct plugin_manager_iterate_cb_type args = {
		manager,
		iterator,
		data
	};

	set_iterate(manager->plugins, &plugin_manager_iterate_cb, (void *)&args);
}

int plugin_manager_clear(plugin_manager manager, plugin p)
{
	const char *name = plugin_name(p);

	if (set_get(manager->plugins, (set_key)name) == NULL)
	{
		return 0;
	}

	if (set_remove(manager->plugins, (const set_key)name) == NULL)
	{
		return 1;
	}

	return 0;
}

int plugin_manager_destroy_cb(set s, set_key key, set_value val, set_cb_iterate_args args)
{
	(void)s;
	(void)key;

	if (val != NULL && args != NULL)
	{
		plugin p = (plugin)val;
		plugin_manager manager = (plugin_manager)args;

		if (manager->iface != NULL && manager->iface->clear != NULL)
		{
			int result = manager->iface->clear(manager, p);

			plugin_destroy(p)

				return result;
		}
	}

	return 0;
}

void plugin_manager_destroy(plugin_manager manager)
{
	/* If there's a destroy callback, probably the plugin manager needs a complex destroy algorithm */
	if (manager->iface != NULL && manager->iface->destroy != NULL)
	{
		manager->iface->destroy(manager, manager->impl);
	}
	else
	{
		/* Otherwise destroy it in an unordered manner */
		if (manager->plugins != NULL)
		{
			set_iterate(manager->plugins, &plugin_manager_destroy_cb, NULL);
		}
	}

	/* Destroy the plugin set */
	if (manager->plugins != NULL)
	{
		set_destroy(manager->plugins);
		manager->plugins = NULL;
	}

	/* Clear the library path */
	if (manager->library_path != NULL)
	{
		environment_variable_path_destroy(manager->library_path);
		manager->library_path = NULL;
	}

	/* Nullify the rest of parameters that do not need deallocation */
	manager->iface = NULL;
	manager->impl = NULL;
	manager->type = NULL;
	manager->environment_library_path = NULL;
	manager->default_library_path = NULL;
}
