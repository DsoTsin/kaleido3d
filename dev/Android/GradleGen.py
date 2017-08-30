import argparse

single_app_proj_src = '''
apply plugin: 'com.android.application'

android {
    compileSdkVersion {android_sdk_level}
    buildToolsVersion "{android_build_tool}"
    defaultConfig {
        applicationId "{application_name}"
        minSdkVersion 19
        targetSdkVersion {target_android_sdk_level}
        versionCode 1
        versionName "1.0"
        externalNativeBuild {
            cmake {
                arguments '-DANDROID_PLATFORM=android-{android_ndk_lebvel}',
                        '-DCMAKE_BUILD_TYPE=Debug',
                        '-DANDROID_TOOLCHAIN=clang',
                        '-DANDROID_STL=gnustl_shared',
                        '-DANDROID_CPP_FEATURES=rtti'
                targets {jni_targets}
            }
        }
        ndk {
            abiFilters {build_abis}
        }
    }

    sourceSets {
        main {
            assets.srcDirs {asset_dir}
        }
    }

    buildTypes {
        release {
            minifyEnabled false
            proguardFiles getDefaultProguardFile('proguard-android.txt'), 'proguard-rules.pro'
        }
    }
    externalNativeBuild {
        cmake {
            path {cmakelists_path}
        }
    }
}

dependencies {
    compile fileTree(dir: 'libs', include: ['*.jar'])
//    compile project(':Project')
    {depend_android_projs}
}
'''


def new_build_gradle(proj, directory, target_type, java_src, cmake_path):
    pass

def new_setting_gradle(solution):
    pass

def copy_gradle_wrapper():
    pass

def find_existing_gradle(path):
    pass