{
  'variables': {
    'project_name': 'exo_browser',
    'product_name': 'ExoBrowser',
    'app_sources': [
      'src/browser/resources/win/exo_browser.rc',
      'src/browser/resources/win/resource.h',
      'common/main.cc',
    ],
    'lib_sources': [
      'src/browser/browser_client.cc',
      'src/browser/browser_client.h',
      'src/browser/browser_main_parts.cc',
      'src/browser/browser_main_parts.h',
      'src/common/main_delegate.cc',
      'src/common/main_delegate.h',
      'src/renderer/renderer_client.cc',
      'src/renderer/renderer_client.h',
      'src/renderer/render_process_observer.cc',
      'src/renderer/render_process_observer.h',
      'src/renderer/render_view_observer.cc',
      'src/renderer/render_view_observer.h',
    ],
    'framework_sources': [
      'src/common/library_main.cc',
      'src/common/library_main.h',
    ],
    'conditions': [
      ['OS=="win"', {
        'app_sources': [
          '<(libchromiumcontent_src_dir)/content/app/startup_helper_win.cc',
        ],
      }],
    ],
  },
  'includes': [
    'vendor/brightray/brightray.gypi',
  ],
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
            'LD_RUNPATH_SEARCH_PATHS': '@executable_path/../Frameworks',
          },
          'copies': [
            {
              'destination': '<(PRODUCT_DIR)/<(product_name).app/Contents/Frameworks',
              'files': [
                '<(PRODUCT_DIR)/<(product_name) Helper.app',
                '<(PRODUCT_DIR)/<(product_name).framework',
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
          ]
        }],
        ['OS=="win"', {
          'copies': [
            {
              'destination': '<(PRODUCT_DIR)',
              'files': [
                '<(libchromiumcontent_library_dir)/chromiumcontent.dll',
                '<(libchromiumcontent_library_dir)/content_shell.pak',
                '<(libchromiumcontent_library_dir)/icudtl.dat',
                '<(libchromiumcontent_library_dir)/libGLESv2.dll',
              ],
            },
          ],
        }],
      ],
    },
    {
      'target_name': '<(project_name)_lib',
      'type': 'static_library',
      'dependencies': [
        'vendor/brightray/brightray.gyp:brightray',
      ],
      'sources': [
        '<@(lib_sources)',
      ],
      'include_dirs': [
        '.',
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
        ['OS!="linux"', {
          'sources/': [
            ['exclude', '/linux/'],
            ['exclude', '_linux\.(cc|h)$'],
          ],
        }],
        ['OS!="mac"', {
          'sources/': [
            ['exclude', '/mac/'],
            ['exclude', '_mac\.(mm|h)$'],
          ],
        },{
          'sources/': [
            ['exclude', '/views/'],
            ['exclude', '_views\.(cc|h)$'],
          ],
        }],
        ['OS!="win"', {
          'sources/': [
            ['exclude', '/win/'],
            ['exclude', '_win\.(cc|h)$'],
          ],
        }],
      ],
    },
  ],
  'conditions': [
    ['OS=="mac"', {
      'targets': [
        {
          'target_name': '<(project_name)_framework',
          'product_name': '<(product_name)',
          'type': 'shared_library',
          'dependencies': [
            '<(project_name)_lib',
          ],
          'sources': [
            '<@(framework_sources)',
          ],
          'mac_bundle': 1,
          'mac_bundle_resources': [
            '<(libchromiumcontent_resources_dir)/content_shell.pak',
            '<(libchromiumcontent_resources_dir)/icudtl.dat',
          ],
          'xcode_settings': {
            'LIBRARY_SEARCH_PATHS': '<(libchromiumcontent_library_dir)',
            'LD_DYLIB_INSTALL_NAME': '@rpath/<(product_name).framework/<(product_name)',
            'LD_RUNPATH_SEARCH_PATHS': '@loader_path/Libraries',
            'OTHER_LDFLAGS': [
              '-ObjC',
            ],
          },
          'copies': [
            {
              'destination': '<(PRODUCT_DIR)/<(product_name).framework/Versions/A/Libraries',
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
                '<(product_name)',
                'Libraries',
                'Frameworks',
              ],
            },
          ],
          'export_dependent_settings': [
            '<(project_name)_lib',
          ],
        },
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
          'mac_bundle': 1,
          'xcode_settings': {
            'INFOPLIST_FILE': 'src/renderer/resources/mac/Info.plist',
            'LD_RUNPATH_SEARCH_PATHS': '@executable_path/../../../../Frameworks',
          },
        },
      ],
    }],
  ],
}
