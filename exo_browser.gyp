# Copyright (c) 2013 Stanislas Polu. All rights reserved.

{
  'variables': {
    'exo_browser_product_name': 'ExoBrowser',
    'exo_browser_app_code': 'app/',
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
        'EXO_BROWSER_APP_CODE="<(exo_browser_app_code)"',
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
        '<(DEPTH)/third_party/node/deps/cares/include',
      ],
      'sources': [
        'src/app/paths_mac.h',
        'src/app/paths_mac.mm',
        'src/app/main_delegate.cc',
        'src/app/main_delegate.h',
        'src/app/main_mac.h',
        'src/app/main_mac.mm',
        'src/common/content_client.cc',
        'src/common/content_client.h',
        'src/common/messages.cc',
        'src/common/messages.h',
        'src/common/switches.cc',
        'src/common/switches.h',
        'src/renderer/content_renderer_client.cc',
        'src/renderer/content_renderer_client.h',
        'src/renderer/render_process_observer.cc',
        'src/renderer/render_process_observer.h',
        'src/renderer/render_view_observer.cc',
        'src/renderer/render_view_observer.h',
        'src/browser/content_browser_client.cc',
        'src/browser/content_browser_client.h',
        'src/browser/browser_application_mac.h',
        'src/browser/browser_application_mac.mm',
        'src/browser/browser_context.cc',
        'src/browser/browser_context.h',
        'src/browser/browser_main_parts.cc',
        'src/browser/browser_main_parts.h',
        'src/browser/browser_main_parts_mac.mm',
        'src/browser/content_browser_client.cc',
        'src/browser/content_browser_client.h',
        'src/browser/download_manager_delegate.cc',
        'src/browser/download_manager_delegate.h',
        'src/browser/resource_dispatcher_host_delegate.cc',
        'src/browser/resource_dispatcher_host_delegate.h',
        'src/browser/ui/exo_browser.h',
        'src/browser/ui/exo_browser.cc',
        'src/browser/ui/exo_browser_gtk.cc',
        'src/browser/ui/exo_browser_mac.mm',
        'src/browser/ui/exo_frame.h',
        'src/browser/ui/exo_frame.cc',
        'src/browser/ui/exo_frame_gtk.cc',
        'src/browser/ui/web_contents_view_delegate.h',
        'src/browser/ui/web_contents_view_delegate_gtk.cc',
        'src/browser/ui/web_contents_view_delegate_mac.mm',
        'src/browser/ui/dialog/javascript_dialog_manager.cc',
        'src/browser/ui/dialog/javascript_dialog_manager.h',
        'src/browser/ui/dialog/javascript_dialog_gtk.cc',
        'src/browser/ui/dialog/javascript_dialog_mac.mm',
        'src/browser/ui/dialog/javascript_dialog_win.cc',
        'src/browser/ui/dialog/javascript_dialog.h',
        'src/devtools/devtools_delegate.cc',
        'src/devtools/devtools_delegate.h',
        'src/devtools/devtools_frontend.cc',
        'src/devtools/devtools_frontend.h',
        'src/net/net_log.cc',
        'src/net/net_log.h',
        'src/net/network_delegate.cc',
        'src/net/network_delegate.h',
        'src/net/url_request_context_getter.cc',
        'src/net/url_request_context_getter.h',
        'src/node/node_thread.h',
        'src/node/node_thread.cc',
        'src/node/api/api_bindings.h',
        'src/node/api/api_bindings.cc',
        'src/node/api/object_wrap.h',
        'src/node/api/object_wrap.cc',
        'src/node/api/exo_browser_wrap.h',
        'src/node/api/exo_browser_wrap.cc',
        'src/node/api/exo_frame_wrap.h',
        'src/node/api/exo_frame_wrap.cc',
        'src/geolocation/access_token_store.cc',
        'src/geolocation/access_token_store.h',
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
      'target_name': 'exo_browser',
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
        'src/app/exo_browser_main.cc',
      ],
      'mac_bundle_resources': [
        'src/resources/app.icns',
        'src/app/app-Info.plist',
      ],
      # TODO(mark): Come up with a fancier way to do this.  It should only
      # be necessary to list app-Info.plist once, not the three times it is
      # listed here.
      'mac_bundle_resources!': [
        'src/app/app-Info.plist',
      ],
      'xcode_settings': {
        'INFOPLIST_FILE': 'src/app/app-Info.plist',
      },
      'msvs_settings': {
        'VCLinkerTool': {
          'SubSystem': '2',  # Set /SUBSYSTEM:WINDOWS
        },
        'VCManifestTool': {
          'AdditionalManifestFiles': 'src/app/exo.exe.manifest',
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
            'src/app/shell.rc',
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
                '<(exo_browser_app_code)'
              ],
            },
          ],
        }], # OS=="linux"
        ['OS=="mac"', {
          'product_name': '<(exo_browser_product_name)',
          'dependencies!': [
            'exo_browser_lib',
          ],
          'dependencies': [
            'exo_browser_framework',
            'exo_browser_helper_app',
          ],
          'copies': [
            {
              'destination': '<(PRODUCT_DIR)/<(exo_browser_product_name).app/Contents/Frameworks',
              'files': [
                '<(PRODUCT_DIR)/<(exo_browser_product_name) Helper.app',
              ],
            }, 
            {
              'destination': '<(PRODUCT_DIR)/<(exo_browser_product_name).app/Contents/Resources',
              'files': [
                '<(exo_browser_app_code)'
              ],
            },
          ],
          'postbuilds': [
            {
              'postbuild_name': 'Copy <(exo_browser_product_name) Framework.framework',
              'action': [
                '<(DEPTH)/build/mac/copy_framework_unversioned.sh',
                '${BUILT_PRODUCTS_DIR}/<(exo_browser_product_name) Framework.framework',
                '${BUILT_PRODUCTS_DIR}/${CONTENTS_FOLDER_PATH}/Frameworks',
              ],
            },
            {
              'postbuild_name': 'Fix Framework Link',
              'action': [
                'install_name_tool',
                '-change',
                '/Library/Frameworks/<(exo_browser_product_name) Framework.framework/Versions/A/<(exo_browser_product_name) Framework',
                '@executable_path/../Frameworks/<(exo_browser_product_name) Framework.framework/<(exo_browser_product_name) Framework',
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
                '<(exo_browser_product_name)',
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
          'product_name': '<(exo_browser_product_name) Framework',
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
            'src/app/main_mac.mm',
            'src/app/main_mac.h',
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
          'product_name': '<(exo_browser_product_name) Helper',
          'mac_bundle': 1,
          'dependencies': [
            'exo_browser_framework',
          ],
          'sources': [
            'src/app/exo_browser_main.cc',
            'src/app/helper-Info.plist',
          ],
          # TODO(mark): Come up with a fancier way to do this.  It should only
          # be necessary to list helper-Info.plist once, not the three times it
          # is listed here.
          'mac_bundle_resources!': [
            'src/app/helper-Info.plist',
          ],
          # TODO(mark): For now, don't put any resources into this app.  Its
          # resources directory will be a symbolic link to the browser app's
          # resources directory.
          'mac_bundle_resources/': [
            ['exclude', '.*'],
          ],
          'xcode_settings': {
            'INFOPLIST_FILE': 'src/app/helper-Info.plist',
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
                '/Library/Frameworks/<(exo_browser_product_name) Framework.framework/Versions/A/<(exo_browser_product_name) Framework',
                '@executable_path/../../../../Frameworks/<(exo_browser_product_name) Framework.framework/<(exo_browser_product_name) Framework',
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

