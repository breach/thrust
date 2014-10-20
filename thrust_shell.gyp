# Copyright (c) 2014 Stanislas Polu. All rights reserved.
# see LICENSE file

{
  'variables': {
    'project_name': 'thrust_shell',
    'product_name': 'ThrustShell',
    'framework_name': 'ThrustShell Framework',
    'app_sources': [
      'src/browser/resources/win/thrust_shell.rc',
      'src/browser/resources/win/resource.h',
      'src/app/main.cc',
      'src/app/main.h',
    ],
    'bundle_sources': [
      'src/browser/resources/mac/thrust_shell.icns',
    ],
    'js_sources': [
      'src/renderer/extensions/resources/web_view.js',
    ],
    'lib_sources': [
      'src/app/main_delegate.cc',
      'src/app/main_delegate.h',
      'src/app/main_delegate_mac.mm',
      'src/app/content_client.h',
      'src/app/content_client.cc',

      'src/common/messages.cc',
      'src/common/messages.h',
      'src/common/switches.cc',
      'src/common/switches.h',
      'src/common/draggable_region.h',
      'src/common/draggable_region.cc',

      'src/browser/resources/linux/application_info_linux.cc',
      'src/browser/browser_client.cc',
      'src/browser/browser_client.h',
      'src/browser/browser_main_parts.cc',
      'src/browser/browser_main_parts.h',
      'src/browser/browser_main_parts_mac.mm',
      'src/browser/util/platform_util.h',
      'src/browser/util/platform_util_aura.cc',
      'src/browser/util/platform_util_linux.cc',
      'src/browser/util/platform_util_mac.mm',
      'src/browser/mac/application_mac.h',
      'src/browser/mac/application_mac.mm',
      'src/browser/resource_dispatcher_host_delegate.cc',
      'src/browser/resource_dispatcher_host_delegate.h',
      'src/browser/session/thrust_session.h',
      'src/browser/session/thrust_session.cc',
      'src/browser/session/thrust_session_cookie_store.h',
      'src/browser/session/thrust_session_cookie_store.cc',
      'src/browser/session/thrust_session_visitedlink_store.h',
      'src/browser/session/thrust_session_visitedlink_store.cc',
      'src/browser/thrust_window.h',
      'src/browser/thrust_window.cc',
      'src/browser/thrust_window_views.cc',
      'src/browser/thrust_window_mac.mm',
      'src/browser/thrust_menu.h',
      'src/browser/thrust_menu.cc',
      'src/browser/thrust_menu_views.cc',
      'src/browser/thrust_menu_mac.mm',
      'src/browser/dialog/javascript_dialog_manager.cc',
      'src/browser/dialog/javascript_dialog_manager.h',
      'src/browser/dialog/file_select_helper.h',
      'src/browser/dialog/file_select_helper.cc',
      'src/browser/dialog/download_manager_delegate.h',
      'src/browser/dialog/download_manager_delegate.cc',
      'src/browser/dialog/download_manager_delegate_gtk.cc',
      'src/browser/dialog/download_manager_delegate_mac.mm',
      'src/browser/ui/accelerator_util.h',
      'src/browser/ui/accelerator_util.cc',
      'src/browser/ui/accelerator_util_mac.mm',
      'src/browser/ui/accelerator_util_views.cc',
      'src/browser/ui/views/frameless_view.h',
      'src/browser/ui/views/frameless_view.cc',
      'src/browser/ui/views/win_frame_view.h',
      'src/browser/ui/views/win_frame_view.cc',

      'src/browser/ui/views/global_menu_bar_x11.h',
      'src/browser/ui/views/global_menu_bar_x11.cc',
      'src/browser/ui/views/global_menu_bar_registrar_x11.h',
      'src/browser/ui/views/global_menu_bar_registrar_x11.cc',
      'src/browser/ui/views/menu_bar.h',
      'src/browser/ui/views/menu_bar.cc',
      'src/browser/ui/views/menu_layout.h',
      'src/browser/ui/views/menu_layout.cc',
      'src/browser/ui/views/menu_delegate.h',
      'src/browser/ui/views/menu_delegate.cc',
      'src/browser/ui/views/submenu_button.h',
      'src/browser/ui/views/submenu_button.cc',

      'src/browser/ui/cocoa/menu_controller.h',
      'src/browser/ui/cocoa/menu_controller.mm',
      'src/browser/ui/cocoa/event_processing_window.h',
      'src/browser/ui/cocoa/event_processing_window.mm',

      'src/browser/web_view/web_view_guest.h',
      'src/browser/web_view/web_view_guest.cc',
      'src/browser/web_view/web_view_constants.h',
      'src/browser/web_view/web_view_constants.cc',

      'src/renderer/renderer_client.cc',
      'src/renderer/renderer_client.h',
      'src/renderer/render_process_observer.cc',
      'src/renderer/render_process_observer.h',
      'src/renderer/render_view_observer.cc',
      'src/renderer/render_view_observer.h',

      'src/renderer/extensions/scoped_persistent.h',
      'src/renderer/extensions/unsafe_persistent.h',
      'src/renderer/extensions/static_v8_external_string_resource.h',
      'src/renderer/extensions/static_v8_external_string_resource.cc',
      'src/renderer/extensions/local_source_map.h',
      'src/renderer/extensions/local_source_map.cc',
      'src/renderer/extensions/native_handler.h',
      'src/renderer/extensions/native_handler.cc',
      'src/renderer/extensions/object_backed_native_handler.cc',
      'src/renderer/extensions/object_backed_native_handler.h',
      'src/renderer/extensions/document_custom_bindings.h',
      'src/renderer/extensions/document_custom_bindings.cc',
      'src/renderer/extensions/safe_builtins.h',
      'src/renderer/extensions/safe_builtins.cc',
      'src/renderer/extensions/console.h',
      'src/renderer/extensions/console.cc',
      'src/renderer/extensions/module_system.h',
      'src/renderer/extensions/module_system.cc',
      'src/renderer/extensions/script_context.h',
      'src/renderer/extensions/script_context.cc',
      'src/renderer/extensions/dispatcher.h',
      'src/renderer/extensions/dispatcher.cc',

      'src/geolocation/access_token_store.cc',
      'src/geolocation/access_token_store.h',

      'src/devtools/devtools_delegate.cc',
      'src/devtools/devtools_delegate.h',

      'src/net/net_log.cc',
      'src/net/net_log.h',
      'src/net/network_delegate.cc',
      'src/net/network_delegate.h',
      'src/net/url_request_context_getter.cc',
      'src/net/url_request_context_getter.h',

      'src/common/visitedlink/visitedlink_common.cc',
      'src/common/visitedlink/visitedlink_common.h',
      'src/common/visitedlink/visitedlink_message_generator.cc',
      'src/common/visitedlink/visitedlink_message_generator.h',
      'src/common/visitedlink/visitedlink_messages.h',
      'src/browser/visitedlink/visitedlink_delegate.h',
      'src/browser/visitedlink/visitedlink_event_listener.cc',
      'src/browser/visitedlink/visitedlink_event_listener.h',
      'src/browser/visitedlink/visitedlink_master.cc',
      'src/browser/visitedlink/visitedlink_master.h',
      'src/renderer/visitedlink/visitedlink_slave.cc',
      'src/renderer/visitedlink/visitedlink_slave.h',

      'src/api/api.h',
      'src/api/api.cc',
      'src/api/api_server.h',
      'src/api/api_server.cc',
      'src/api/api_binding.h',
      'src/api/api_binding.cc',
      'src/api/thrust_window_binding.h',
      'src/api/thrust_window_binding.cc',
      'src/api/thrust_session_binding.h',
      'src/api/thrust_session_binding.cc',
      'src/api/thrust_menu_binding.h',
      'src/api/thrust_menu_binding.cc',
    ],
    'framework_sources': [
      'src/app/library_main.cc',
      'src/app/library_main.h',
    ],
    'locales': [
      'am', 'ar', 'bg', 'bn', 'ca', 'cs', 'da', 'de', 'el', 'en-GB',
      'en-US', 'es-419', 'es', 'et', 'fa', 'fi', 'fil', 'fr', 'gu', 'he',
      'hi', 'hr', 'hu', 'id', 'it', 'ja', 'kn', 'ko', 'lt', 'lv',
      'ml', 'mr', 'ms', 'nb', 'nl', 'pl', 'pt-BR', 'pt-PT', 'ro', 'ru',
      'sk', 'sl', 'sr', 'sv', 'sw', 'ta', 'te', 'th', 'tr', 'uk',
      'vi', 'zh-CN', 'zh-TW',
    ],
    'thrust_shell_source_root': '<!(python tools/source_root.py)',
    'conditions': [
      ['OS=="win"', {
        'app_sources': [
          'src/browser/resources/win/resource.h',
          'src/browser/resources/win/thrust_shell.ico',
          'src/browser/resources/win/thrust_shell.rc',
          '<(libchromiumcontent_src_dir)/content/app/startup_helper_win.cc',
        ],
      }],  # OS=="win"
    ],
  },
  'includes': [
    'common.gypi',
    'vendor/brightray/brightray.gypi',
  ],
  'target_defaults': {
    'includes': [
       # Rules for excluding e.g. foo_win.cc from the build on non-Windows.
      'filename_rules.gypi',
    ],
    'configurations': {
      'Debug': {
        'defines': [ 'DEBUG' ],
        'cflags': [ '-g', '-O0' ],
      },
    },
  },
  'targets': [
    {
      'target_name': '<(project_name)',
      'type': 'executable',
      'dependencies': [
        '<(project_name)_lib',
      ],
      'sources': [
        '<@(app_sources)',
      ],
      'include_dirs': [
        '.',
      ],
      'conditions': [
        ['OS=="mac"', {
          'product_name': '<(product_name)',
          'mac_bundle': 1,
          'dependencies!': [
            '<(project_name)_lib',
          ],
          'dependencies': [
            '<(project_name)_framework',
            '<(project_name)_helper',
          ],
          'xcode_settings': {
            'INFOPLIST_FILE': 'src/browser/resources/mac/Info.plist',
            'LD_RUNPATH_SEARCH_PATHS': [
              '@executable_path/../Frameworks',
            ],
          },
          'mac_bundle_resources': [
            '<@(bundle_sources)',
            '<(libchromiumcontent_resources_dir)/icudtl.dat',
          ],
          'copies': [
            {
              'destination': '<(PRODUCT_DIR)/<(product_name).app/Contents/Frameworks',
              'files': [
                '<(PRODUCT_DIR)/<(product_name) Helper.app',
                '<(PRODUCT_DIR)/<(framework_name).framework',
              ],
            },
          ],
          'postbuilds': [
            {
              # This postbuid step is responsible for creating the following
              # helpers:
              #
              # <(product_name) EH.app and <(product_name) NP.app are created
              # from <(product_name).app.
              #
              # The EH helper is marked for an executable heap. The NP helper
              # is marked for no PIE (ASLR).
              'postbuild_name': 'Make More Helpers',
              'action': [
                'vendor/brightray/tools/mac/make_more_helpers.sh',
                'Frameworks',
                '<(product_name)',
              ],
            },
            {
              # The application doesn't have real localizations, it just has
              # empty .lproj directories, which is enough to convince Cocoa
              # thrust supports those languages.
              'postbuild_name': 'Make Empty Localizations',
              'variables': {
                'locale_dirs': [
                  '>!@(<(apply_locales_cmd) -d ZZLOCALE.lproj <(locales))',
                ],
              },
              'action': [
                'tools/mac/make_locale_dirs.sh',
                '<@(locale_dirs)',
              ],
            },
          ]
        }, {  # OS=="mac"
          'dependencies': [
            'make_locale_paks',
          ],
        }],  # OS!="mac"
        ['OS=="win"', {
          'copies': [
            {
              'destination': '<(PRODUCT_DIR)',
              'files': [
                '<(libchromiumcontent_library_dir)/chromiumcontent.dll',
                '<(libchromiumcontent_library_dir)/ffmpegsumo.dll',
                '<(libchromiumcontent_library_dir)/libEGL.dll',
                '<(libchromiumcontent_library_dir)/libGLESv2.dll',
                '<(libchromiumcontent_resources_dir)/icudtl.dat',
                '<(libchromiumcontent_resources_dir)/content_shell.pak',
                '<(libchromiumcontent_resources_dir)/ui_resources_200_percent.pak',
                '<(libchromiumcontent_resources_dir)/webkit_resources_200_percent.pak',
              ],
            },
          ],
        }],
        ['OS=="linux"', {
          'link_settings': {
            'libraries': [
              '-lpthread -ldl -lm -lX11 -lXrandr -lXext -lgconf-2',
            ],
          },
          'copies': [
            {
              'destination': '<(PRODUCT_DIR)',
              'files': [
                '<(libchromiumcontent_library_dir)/libchromiumcontent.so',
                '<(libchromiumcontent_library_dir)/libffmpegsumo.so',
                '<(libchromiumcontent_resources_dir)/icudtl.dat',
                '<(libchromiumcontent_resources_dir)/content_shell.pak',
              ],
            },
          ],
        }],
      ],
    },  # target <(project_name)
    {
      'target_name': '<(project_name)_lib',
      'type': 'static_library',
      'dependencies': [
        '<(project_name)_js',
        'vendor/brightray/brightray.gyp:brightray',
      ],
      'defines': [
        # This is defined in skia/skia_common.gypi.
        'SK_SUPPORT_LEGACY_GETTOPDEVICE',
      ],
      'sources': [
        '<@(lib_sources)',
      ],
      'include_dirs': [
        '.',
        'chromium_src',
        'vendor/brightray',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '.',
        ],
      },
      'export_dependent_settings': [
        'vendor/brightray/brightray.gyp:brightray',
      ],
      'conditions': [
        ['OS=="win"', {
          'link_settings': {
            'libraries': [
              '-limm32.lib',
              '-loleacc.lib',
              '-lComdlg32.lib',
              '-lWininet.lib',
            ],
          },
        }],  # OS=="win"
        ['OS=="mac"', {
          'dependencies': [
          ],
        }],  # OS=="mac"
         ['OS=="linux"', {
           'link_settings': {
             'ldflags': [
              # Make binary search for libraries under current directory, so we
              # don't have to manually set $LD_LIBRARY_PATH:
              # http://serverfault.com/questions/279068/cant-find-so-in-the-same-directory-as-the-executable
               '-rpath \$$ORIGIN',
              # Make native module dynamic loading work.
               '-rdynamic',
             ],
           },
          'cflags': [
            '-Wno-deprecated-register',
            '-Wno-empty-body',
          ],
        }],  # OS=="linux"
      ],
    },  # target <(product_name)_lib
    {
      'target_name': '<(project_name)_js',
      'type': 'none',
      'actions': [
        {
          'inputs': [
            'src/renderer/extensions/resources/web_view.js',
          ],
          'outputs': [
            'src/renderer/extensions/resources/web_view.js.bin',
          ],
          'action_name': 'xxd web_view.js',
          'action': ['xxd', '-i', 
          'src/renderer/extensions/resources/web_view.js',
          'src/renderer/extensions/resources/web_view.js.bin'],
        },
      ],
    },  # target <(product_name)_js
    {
      'target_name': '<(project_name)_strip',
      'type': 'none',
      'dependencies': [
        '<(project_name)',
      ],
      'conditions': [
        ['OS=="linux"', {
          'actions': [
            {
              'action_name': 'Strip Binary',
              'inputs': [
                '<(PRODUCT_DIR)/libchromiumcontent.so',
                '<(PRODUCT_DIR)/libffmpegsumo.so',
                '<(PRODUCT_DIR)/<(project_name)',
              ],
              'outputs': [
                # Gyp action requires a output file, add a fake one here.
                '<(PRODUCT_DIR)/dummy_file',
              ],
              'action': [
                'tools/posix/strip.sh',
                '<@(_inputs)',
              ],
            },
          ],
        }],  # OS=="linux"
      ],
    },  # target <(project_name>_strip
  ],
  'conditions': [
    ['OS=="mac"', {
      'targets': [
        {
          'target_name': '<(project_name)_framework',
          'product_name': '<(framework_name)',
          'type': 'shared_library',
          'dependencies': [
            '<(project_name)_lib',
          ],
          'sources': [
            '<@(framework_sources)',
          ],
          'include_dirs': [
            '.',
            'vendor',
            '<(libchromiumcontent_include_dir)',
          ],
          'export_dependent_settings': [
            '<(project_name)_lib',
          ],
          'link_settings': {
            'libraries': [
              '$(SDKROOT)/System/Library/Frameworks/Carbon.framework',
              '$(SDKROOT)/System/Library/Frameworks/QuartzCore.framework',
            ],
          },
          'mac_bundle': 1,
          'mac_bundle_resources': [
            '<(libchromiumcontent_resources_dir)/content_shell.pak',
            '<(libchromiumcontent_resources_dir)/icudtl.dat',
          ],
          'xcode_settings': {
            'INFOPLIST_FILE': 'src/common/resources/mac/Info.plist',
            'LIBRARY_SEARCH_PATHS': [
              '<(libchromiumcontent_library_dir)',
            ],
            'LD_DYLIB_INSTALL_NAME': '@rpath/<(framework_name).framework/<(framework_name)',
            'LD_RUNPATH_SEARCH_PATHS': [
              '@loader_path/Libraries',
            ],
            'OTHER_LDFLAGS': [
              '-ObjC',
            ],
          },
          'copies': [
            {
              'destination': '<(PRODUCT_DIR)/<(framework_name).framework/Versions/A/Libraries',
              'files': [
                '<(libchromiumcontent_library_dir)/ffmpegsumo.so',
                '<(libchromiumcontent_library_dir)/libchromiumcontent.dylib',
              ],
            },
          ],
          'postbuilds': [
            {
              'postbuild_name': 'Add symlinks for framework subdirectories',
              'action': [
                'tools/mac/create-framework-subdir-symlinks.sh',
                '<(framework_name)',
                'Libraries',
                'Frameworks',
              ],
            },
          ],
        },  # target framework
        {
          'target_name': '<(project_name)_helper',
          'product_name': '<(product_name) Helper',
          'type': 'executable',
          'dependencies': [
            '<(project_name)_framework',
          ],
          'sources': [
            '<@(app_sources)',
          ],
          'include_dirs': [
            '.',
          ],
          'mac_bundle': 1,
          'xcode_settings': {
            'INFOPLIST_FILE': 'src/renderer/resources/mac/Info.plist',
            'LD_RUNPATH_SEARCH_PATHS': [
              '@executable_path/../../..',
            ],
          },
        },  # target helper
      ],
    }, {  # OS=="mac"
      'targets': [
        {
          'target_name': 'make_locale_paks',
          'type': 'none',
          'actions': [
            {
              'action_name': 'Make Empty Paks',
              'inputs': [
                'tools/posix/make_locale_paks.sh',
              ],
              'outputs': [
                '<(PRODUCT_DIR)/locales'
              ],
              'action': [
                'tools/posix/make_locale_paks.sh',
                '<(PRODUCT_DIR)',
                '<@(locales)',
              ],
              'msvs_cygwin_shell': 0,
            },
          ],
        },
      ],
    }],  # OS!="mac"
  ],
}
