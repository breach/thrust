# Copyright (c) 2013 Stanislas Polu. All rights reserved.

{
  'variables': {
    'exo_product_name': 'Exo',
    'exo_app_code': 'app/',
    'exo_browser_version': '0.1.0.0',
    'ua_version': '28.0.1500.95',
  },
  'targets': [
    {
      'target_name': 'exo_browser_lib',
      'type': 'static_library',
      'defines!': ['CONTENT_IMPLEMENTATION'],
      'defines': [
        'EXO_BROWSER_VERSION="<(exo_browser_version)"',
        'UA_VERSION="<(ua_version)"',
        'EXO_APP_CODE="<(exo_app_code)"',
      ],
      'variables': {
        'chromium_code': 1,
      },
      'dependencies': [
        'exo_browser_resources',
        '<(DEPTH)/content/content.gyp:content_app_both',
        '<(DEPTH)/content/content.gyp:content_browser',
        '<(DEPTH)/content/content.gyp:content_common',
        '<(DEPTH)/content/content.gyp:content_gpu',
        '<(DEPTH)/content/content.gyp:content_plugin',
        '<(DEPTH)/content/content.gyp:content_ppapi_plugin',
        '<(DEPTH)/content/content.gyp:content_renderer',
        '<(DEPTH)/content/content.gyp:content_utility',
        '<(DEPTH)/content/content.gyp:content_worker',
        '<(DEPTH)/content/content_resources.gyp:content_resources',
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        '<(DEPTH)/ipc/ipc.gyp:ipc',
        '<(DEPTH)/media/media.gyp:media',
        '<(DEPTH)/net/net.gyp:net',
        '<(DEPTH)/net/net.gyp:net_resources',
        '<(DEPTH)/skia/skia.gyp:skia',
        '<(DEPTH)/third_party/WebKit/public/blink_test_runner.gyp:blink_test_runner',
        '<(DEPTH)/ui/gl/gl.gyp:gl',
        '<(DEPTH)/ui/ui.gyp:ui',
        '<(DEPTH)/url/url.gyp:url_lib',
        '<(DEPTH)/v8/tools/gyp/v8.gyp:v8',
        '<(DEPTH)/webkit/webkit_resources.gyp:webkit_resources',
        '<(DEPTH)/webkit/support/webkit_support.gyp:webkit_support',
        '<(DEPTH)/third_party/node/node.gyp:node_base'
      ],
      'include_dirs': [
        '<(DEPTH)'
        '<(DEPTH)/third_party/node/src',
        '<(DEPTH)/third_party/node/deps/uv/include',
      ],
      'sources': [
        'exo_browser/app/paths_mac.h',
        'exo_browser/app/paths_mac.mm',
        'exo_browser/app/main_delegate.cc',
        'exo_browser/app/main_delegate.h',
        'exo_browser/app/main_mac.h',
        'exo_browser/app/main_mac.mm',
        'exo_browser/common/content_client.cc',
        'exo_browser/common/content_client.h',
        'exo_browser/common/messages.cc',
        'exo_browser/common/messages.h',
        'exo_browser/common/switches.cc',
        'exo_browser/common/switches.h',
        'exo_browser/renderer/content_renderer_client.cc',
        'exo_browser/renderer/content_renderer_client.h',
        'exo_browser/renderer/render_process_observer.cc',
        'exo_browser/renderer/render_process_observer.h',
        'exo_browser/renderer/render_view_observer.cc',
        'exo_browser/renderer/render_view_observer.h',
        'exo_browser/browser/content_browser_client.cc',
        'exo_browser/browser/content_browser_client.h',
        'exo_browser/browser/browser_application_mac.h',
        'exo_browser/browser/browser_application_mac.mm',
        'exo_browser/browser/browser_context.cc',
        'exo_browser/browser/browser_context.h',
        'exo_browser/browser/browser_main_parts.cc',
        'exo_browser/browser/browser_main_parts.h',
        'exo_browser/browser/browser_main_parts_mac.mm',
        'exo_browser/browser/content_browser_client.cc',
        'exo_browser/browser/content_browser_client.h',
        'exo_browser/browser/download_manager_delegate.cc',
        'exo_browser/browser/download_manager_delegate.h',
        'exo_browser/browser/resource_dispatcher_host_delegate.cc',
        'exo_browser/browser/resource_dispatcher_host_delegate.h',
        'exo_browser/browser/ui/exo_browser.h',
        'exo_browser/browser/ui/exo_browser.cc',
        'exo_browser/browser/ui/exo_browser_gtk.cc',
        'exo_browser/browser/ui/exo_browser_mac.mm',
        'exo_browser/browser/ui/exo_frame.h',
        'exo_browser/browser/ui/exo_frame.cc',
        'exo_browser/browser/ui/exo_frame_gtk.cc',
        'exo_browser/browser/ui/web_contents_view_delegate.h',
        'exo_browser/browser/ui/web_contents_view_delegate_gtk.cc',
        'exo_browser/browser/ui/web_contents_view_delegate_mac.mm',
        'exo_browser/browser/ui/dialog/javascript_dialog_manager.cc',
        'exo_browser/browser/ui/dialog/javascript_dialog_manager.h',
        'exo_browser/browser/ui/dialog/javascript_dialog_gtk.cc',
        'exo_browser/browser/ui/dialog/javascript_dialog_mac.mm',
        'exo_browser/browser/ui/dialog/javascript_dialog_win.cc',
        'exo_browser/browser/ui/dialog/javascript_dialog.h',
        'exo_browser/devtools/devtools_delegate.cc',
        'exo_browser/devtools/devtools_delegate.h',
        'exo_browser/devtools/devtools_frontend.cc',
        'exo_browser/devtools/devtools_frontend.h',
        'exo_browser/net/net_log.cc',
        'exo_browser/net/net_log.h',
        'exo_browser/net/network_delegate.cc',
        'exo_browser/net/network_delegate.h',
        'exo_browser/net/url_request_context_getter.cc',
        'exo_browser/net/url_request_context_getter.h',
        'exo_browser/node/node_thread.h',
        'exo_browser/node/node_thread.cc',
        'exo_browser/node/api/api_bindings.h',
        'exo_browser/node/api/api_bindings.cc',
        'exo_browser/node/api/object_wrap.h',
        'exo_browser/node/api/object_wrap.cc',
        'exo_browser/node/api/exo_browser_wrap.h',
        'exo_browser/node/api/exo_browser_wrap.cc',
        'exo_browser/node/api/exo_frame_wrap.h',
        'exo_browser/node/api/exo_frame_wrap.cc',
        'exo_browser/geolocation/access_token_store.cc',
        'exo_browser/geolocation/access_token_store.h',
      ],
      'msvs_settings': {
        'VCLinkerTool': {
          'SubSystem': '2',  # Set /SUBSYSTEM:WINDOWS
        },
      },
      'conditions': [
        ['OS=="win" and win_use_allocator_shim==1', {
          'dependencies': [
            '<(DEPTH)/base/allocator/allocator.gyp:allocator',
          ],
        }],
        ['toolkit_uses_gtk == 1', {
          'dependencies': [
            # For FT_Init_FreeType and friends.
            '<(DEPTH)/build/linux/system.gyp:freetype2',
            '<(DEPTH)/build/linux/system.gyp:gtk',
            '<(DEPTH)/build/linux/system.gyp:gtkprint',
          ],
        }],
        ['OS=="win"', {
          'resource_include_dirs': [
            '<(SHARED_INTERMEDIATE_DIR)/webkit',
          ],
          'dependencies': [
            '<(DEPTH)/webkit/webkit_resources.gyp:webkit_strings',   
          ],
          'configurations': {
            'Debug_Base': {
              'msvs_settings': {
                'VCLinkerTool': {
                  'LinkIncremental': '<(msvs_large_module_debug_link_mode)',
                },
              },
            },
          },
          'msvs_disabled_warnings': [ 4800 ],
        }], # OS=="win"
        ['OS=="linux"', {
          'dependencies': [
            '<(DEPTH)/build/linux/system.gyp:fontconfig',
            '<(DEPTH)/base/allocator/allocator.gyp:allocator',
          ],
        }], # OS=='linux'
        ['(os_posix==1 and use_aura==1 and linux_use_tcmalloc==1) or (android_use_tcmalloc==1)', {
          'dependencies': [
            # This is needed by content/app/content_main_runner.cc
            '<(DEPTH)/base/allocator/allocator.gyp:allocator',
          ],
        }],
        ['use_aura==1', {
          'dependencies': [
            '<(DEPTH)/ui/aura/aura.gyp:aura',
            '<(DEPTH)/ui/base/strings/ui_strings.gyp:ui_strings',
            '<(DEPTH)/ui/views/controls/webview/webview.gyp:webview',
            '<(DEPTH)/ui/views/views.gyp:views',
            '<(DEPTH)/ui/views/views.gyp:views_test_support',
            '<(DEPTH)/ui/ui.gyp:ui_resources',
          ],
          'sources/': [
            ['exclude', 'browser/ui/browser_gtk.cc'],
            ['exclude', 'browser/ui/browser_win.cc'],
          ],
        }],  # use_aura==1
      ],
    },
    {
      'target_name': 'exo_browser_resources',
      'type': 'none',
      'dependencies': [
        'generate_exo_browser_resources',
      ],
      'variables': {
        'grit_out_dir': '<(SHARED_INTERMEDIATE_DIR)/exo_browser',
      },
      'includes': [ '../build/grit_target.gypi' ],
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)',
          'files': [
            '<(SHARED_INTERMEDIATE_DIR)/exo_browser/exo_browser_resources.pak'
          ],
        },
      ],
    },
    {
      'target_name': 'generate_exo_browser_resources',
      'type': 'none',
      'variables': {
        'grit_out_dir': '<(SHARED_INTERMEDIATE_DIR)/exo_browser',
      },
      'actions': [
        {
          'action_name': 'exo_browser_resources',
          'variables': {
            'grit_grd_file': 'exo_browser_resources.grd',
          },
          'includes': [ '../build/grit_action.gypi' ],
        },
      ],
    },
    {
      # We build a minimal set of resources so WebKit in exo_browser has
      # access to necessary resources.
      'target_name': 'exo_browser_pak',
      'type': 'none',
      'dependencies': [
        '<(DEPTH)/content/browser/devtools/devtools_resources.gyp:devtools_resources',
        '<(DEPTH)/content/content_resources.gyp:content_resources',
        'exo_browser_resources',
        '<(DEPTH)/net/net.gyp:net_resources',
        '<(DEPTH)/ui/base/strings/ui_strings.gyp:ui_strings',
        '<(DEPTH)/ui/ui.gyp:ui_resources',
        '<(DEPTH)/webkit/webkit_resources.gyp:webkit_resources',
        '<(DEPTH)/webkit/webkit_resources.gyp:webkit_strings',
      ],
      'conditions': [
        ['OS!="android" and OS!="ios"', {
          'dependencies': [
            '<(DEPTH)/content/browser/tracing/tracing_resources.gyp:tracing_resources',
          ],
        }],
      ],
      'variables': {
        'repack_path': '<(DEPTH)/tools/grit/grit/format/repack.py',
      },
      'actions': [
        {
          'action_name': 'repack_exo_browser_pack',
          'variables': {
            'pak_inputs': [
              '<(SHARED_INTERMEDIATE_DIR)/content/content_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/exo_browser/exo_browser_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/net/net_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/ui/app_locale_settings/app_locale_settings_en-US.pak',
              '<(SHARED_INTERMEDIATE_DIR)/ui/ui_resources/ui_resources_100_percent.pak',
              '<(SHARED_INTERMEDIATE_DIR)/ui/ui_resources/webui_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/ui/ui_strings/ui_strings_en-US.pak',
              '<(SHARED_INTERMEDIATE_DIR)/webkit/devtools_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/webkit/blink_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/webkit/webkit_resources_100_percent.pak',
              '<(SHARED_INTERMEDIATE_DIR)/webkit/webkit_strings_en-US.pak',
            ],
          },
          'inputs': [
            '<(repack_path)',
            '<@(pak_inputs)',
          ],
          'outputs': [
            '<(PRODUCT_DIR)/exo_browser.pak',
          ],
          'action': ['python', '<(repack_path)', '<@(_outputs)',
                     '<@(pak_inputs)'],
        },
      ],
    },
    {
      'target_name': 'exo',
      'type': 'executable',
      'mac_bundle': 1,
      'defines!': ['CONTENT_IMPLEMENTATION'],
      'variables': {
        'chromium_code': 1,
      },
      'dependencies': [
        'exo_browser_lib',
        'exo_browser_pak',
      ],
      'include_dirs': [
        '<(DEPTH)',
      ],
      'sources': [
        '<(DEPTH)/content/app/startup_helper_win.cc',
        'exo_browser/app/exo_main.cc',
      ],
      'mac_bundle_resources': [
        'exo_browser/resources/app.icns',
        'exo_browser/app/app-Info.plist',
      ],
      # TODO(mark): Come up with a fancier way to do this.  It should only
      # be necessary to list app-Info.plist once, not the three times it is
      # listed here.
      'mac_bundle_resources!': [
        'exo_browser/app/app-Info.plist',
      ],
      'xcode_settings': {
        'INFOPLIST_FILE': 'exo_browser/app/app-Info.plist',
      },
      'msvs_settings': {
        'VCLinkerTool': {
          'SubSystem': '2',  # Set /SUBSYSTEM:WINDOWS
        },
        'VCManifestTool': {
          'AdditionalManifestFiles': 'exo_browser/app/exo.exe.manifest',
        },
      },
      'conditions': [
        ['OS=="win" and win_use_allocator_shim==1', {
          'dependencies': [
            '<(DEPTH)/base/allocator/allocator.gyp:allocator',
          ],
        }],
        ['OS=="win"', {
          'sources': [
            'exo_browser/app/shell.rc',
          ],
          'configurations': {
            'Debug_Base': {
              'msvs_settings': {
                'VCLinkerTool': {
                  'LinkIncremental': '<(msvs_large_module_debug_link_mode)',
                },
              },
            },
          },
        }],  # OS=="win"
        ['OS == "win" or toolkit_uses_gtk == 1', {
          'dependencies': [
            '<(DEPTH)/sandbox/sandbox.gyp:sandbox',
          ],
        }],  # OS=="win" or toolkit_uses_gtk == 1
        ['toolkit_uses_gtk == 1', {
          'dependencies': [
            '<(DEPTH)/build/linux/system.gyp:gtk',
          ],
        }],  # toolkit_uses_gtk
        ['OS=="linux"', {
          'copies': [
            {
              'destination': '<(PRODUCT_DIR)',
              'files': [
                '<(exo_app_code)'
              ],
            },
          ],
        }], # OS=="linux"
        ['OS=="mac"', {
          'product_name': '<(exo_product_name)',
          'dependencies!': [
            'exo_browser_lib',
          ],
          'dependencies': [
            'exo_browser_framework',
            'exo_browser_helper_app',
          ],
          'copies': [
            {
              'destination': '<(PRODUCT_DIR)/<(exo_product_name).app/Contents/Frameworks',
              'files': [
                '<(PRODUCT_DIR)/<(exo_product_name) Helper.app',
              ],
            }, 
            {
              'destination': '<(PRODUCT_DIR)/<(exo_product_name).app/Contents/Resources',
              'files': [
                '<(exo_app_code)'
              ],
            },
          ],
          'postbuilds': [
            {
              'postbuild_name': 'Copy <(exo_product_name) Framework.framework',
              'action': [
                '<(DEPTH)/build/mac/copy_framework_unversioned.sh',
                '${BUILT_PRODUCTS_DIR}/<(exo_product_name) Framework.framework',
                '${BUILT_PRODUCTS_DIR}/${CONTENTS_FOLDER_PATH}/Frameworks',
              ],
            },
            {
              'postbuild_name': 'Fix Framework Link',
              'action': [
                'install_name_tool',
                '-change',
                '/Library/Frameworks/<(exo_product_name) Framework.framework/Versions/A/<(exo_product_name) Framework',
                '@executable_path/../Frameworks/<(exo_product_name) Framework.framework/<(exo_product_name) Framework',
                '${BUILT_PRODUCTS_DIR}/${EXECUTABLE_PATH}'
              ],
            },
            {
              # Modify the Info.plist as needed.
              'postbuild_name': 'Tweak Info.plist',
              'action': ['<(DEPTH)/build/mac/tweak_info_plist.py',
                         '--scm=1',
                         '--version=<(exo_browser_version)'],
            },
            {
              # This postbuid step is responsible for creating the following
              # helpers:
              #
              # Content Shell Helper EH.app and Content Shell Helper NP.app are
              # created from Content Shell Helper.app.
              #
              # The EH helper is marked for an executable heap. The NP helper
              # is marked for no PIE (ASLR).
              'postbuild_name': 'Make More Helpers',
              'action': [
                '<(DEPTH)/build/mac/make_more_helpers.sh',
                'Frameworks',
                '<(exo_product_name)',
              ],
            },
            {
              # Make sure there isn't any Objective-C in the shell's
              # executable.
              'postbuild_name': 'Verify No Objective-C',
              'action': [
                '<(DEPTH)/build/mac/verify_no_objc.sh',
              ],
            },
          ],
        }],  # OS=="mac"
      ],
    },
  ],
  'conditions': [
    ['OS=="mac"', {
      'targets': [
        {
          'target_name': 'exo_browser_framework',
          'type': 'shared_library',
          'product_name': '<(exo_product_name) Framework',
          'mac_bundle': 1,
          'mac_bundle_resources': [
            #'app/English.lproj/HttpAuth.xib',
            #'app/English.lproj/MainMenu.xib',
            '<(PRODUCT_DIR)/exo_browser.pak'
          ],
          'dependencies': [
            'exo_browser_lib',
          ],
          'include_dirs': [
            '<(DEPTH)',
          ],
          'sources': [
            'exo_browser/app/main_mac.mm',
            'exo_browser/app/main_mac.h',
          ],
          'copies': [
            {
              # Copy FFmpeg binaries for audio/video support.
              'destination': '<(PRODUCT_DIR)/$(CONTENTS_FOLDER_PATH)/Libraries',
              'files': [
                '<(PRODUCT_DIR)/ffmpegsumo.so',
              ],
            },
          ],
        },  # target exo_browser_framework
        {
          'target_name': 'exo_browser_helper_app',
          'type': 'executable',
          'variables': { 'enable_wexit_time_destructors': 1, },
          'product_name': '<(exo_product_name) Helper',
          'mac_bundle': 1,
          'dependencies': [
            'exo_browser_framework',
          ],
          'sources': [
            'exo_browser/app/exo_main.cc',
            'exo_browser/app/helper-Info.plist',
          ],
          # TODO(mark): Come up with a fancier way to do this.  It should only
          # be necessary to list helper-Info.plist once, not the three times it
          # is listed here.
          'mac_bundle_resources!': [
            'exo_browser/app/helper-Info.plist',
          ],
          # TODO(mark): For now, don't put any resources into this app.  Its
          # resources directory will be a symbolic link to the browser app's
          # resources directory.
          'mac_bundle_resources/': [
            ['exclude', '.*'],
          ],
          'xcode_settings': {
            'INFOPLIST_FILE': 'exo_browser/app/helper-Info.plist',
          },
          'postbuilds': [
            {
              # The framework defines its load-time path
              # (DYLIB_INSTALL_NAME_BASE) relative to the main executable
              # (chrome).  A different relative path needs to be used in
              # exo_browser_helper_app.
              'postbuild_name': 'Fix Framework Link',
              'action': [
                'install_name_tool',
                '-change',
                '/Library/Frameworks/<(exo_product_name) Framework.framework/Versions/A/<(exo_product_name) Framework',
                '@executable_path/../../../../Frameworks/<(exo_product_name) Framework.framework/<(exo_product_name) Framework',
                '${BUILT_PRODUCTS_DIR}/${EXECUTABLE_PATH}'
              ],
            },
            {
              # Modify the Info.plist as needed.  The script explains why this
              # is needed.  This is also done in the chrome and chrome_dll
              # targets.  In this case, --breakpad=0, --keystone=0, and --scm=0
              # are used because Breakpad, Keystone, and SCM keys are
              # never placed into the helper.
              'postbuild_name': 'Tweak Info.plist',
              'action': ['<(DEPTH)/build/mac/tweak_info_plist.py',
                         '--breakpad=0',
                         '--keystone=0',
                         '--scm=0',
                         '--version=<(exo_browser_version)'],
            },
            {
              # Make sure there isn't any Objective-C in the helper app's
              # executable.
              'postbuild_name': 'Verify No Objective-C',
              'action': [
                '<(DEPTH)/build/mac/verify_no_objc.sh',
              ],
            },
          ],
          'conditions': [
            ['component=="shared_library"', {
              'xcode_settings': {
                'LD_RUNPATH_SEARCH_PATHS': [
                  # Get back from ExoBrowser.app/Contents/Frameworks/
                  #                                 Helper.app/Contents/MacOS
                  '@loader_path/../../../../../..',
                ],
              },
            }],
          ],
        },  # target exo_browser_helper_app
      ],
    }],  # OS=="mac"
  ]
}

