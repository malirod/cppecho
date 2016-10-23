# -*- coding: utf-8 -*-

import os
import contextlib
import shutil
from waflib.Build import (BuildContext, CleanContext, # pylint: disable=import-error
                          InstallContext, UninstallContext)
from waflib.Tools.compiler_cxx import cxx_compiler # pylint: disable=import-error

top = '.' # pylint: disable=invalid-name
out = 'build' # pylint: disable=invalid-name


def options(opt):
    opt.load('compiler_cxx')
    opt.add_option('--sanitize-with',
                   action='store',
                   default=None,
                   dest='sanitize_with',
                   help='Build with sanitizer. Works for debug build.'
                        ' [asan, tsan, msan, ubsan]',
                   choices=('asan', 'tsan', 'msan', 'ubsan'))


def configure(ctx):
    def setup_sanitizer(ctx):
        if ctx.options.sanitize_with == 'asan':
            ctx.env.CXXFLAGS += ['-fsanitize=address']
            ctx.env.LINKFLAGS += ['-fsanitize=address']
        elif ctx.options.sanitize_with == 'tsan':
            ctx.env.CXXFLAGS += ['-fsanitize=thread']
            ctx.env.LINKFLAGS += ['-fsanitize=thread']
        elif ctx.options.sanitize_with == 'msan':
            ctx.env.CXXFLAGS += ['-fsanitize=memory']
            ctx.env.LINKFLAGS += ['-fsanitize=memory']
        elif ctx.options.sanitize_with == 'ubsan':
            ctx.env.CXXFLAGS += ['-fsanitize=undefined']
            ctx.env.LINKFLAGS += ['-fsanitize=undefined']
        ctx.msg('Sanitizer configured ({})'.format(ctx.options.sanitize_with),
                'ok')

    def run_cmd(ctx, cmd_line):
        """ Run command in system shell """
        import subprocess
        print cmd_line
        ret = subprocess.call(cmd_line, shell=True)
        if ret != 0:
            ctx.fatal('Command failed with exit code {}'.format(ret))

    @contextlib.contextmanager
    def working_directory(path):
        """ A context manager which changes the working directory to the given
        path, and then changes it back to it's previous value on exit.
        """
        prev_cwd = os.getcwd()
        os.chdir(path)
        try:
            yield
        finally:
            os.chdir(prev_cwd)

    def setup_submodules(ctx):
        #Setup log4cplus
        log4cplus_home = os.path.join(ctx.path.abspath(), 'thirdparty', 'log4cplus')
        if not os.path.isdir(log4cplus_home):
            print('log4cplus submodule is not found. Running'
                  ' `git submodule update --init --recursive`')
            run_cmd(ctx, 'git submodule update --init --recursive')
        log4cplus_build_dir_name = 'build-{}'.format(ctx.env.CXX.name)
        log4cplus_build_dir = os.path.join(log4cplus_home,
                                           log4cplus_build_dir_name)
        if not os.path.isdir(log4cplus_home):
            ctx.fatal('Failed to init log4cplus')
        if not os.path.isdir(log4cplus_build_dir):
            with working_directory(log4cplus_home):
                run_cmd(ctx, './scripts/fix-timestamps.sh')
                run_cmd(ctx, './configure CXX="{}" --disable-shared'
                             ' --enable-static --prefix=$PWD/{}'.format(ctx.env.CXX[0],
                                                                        log4cplus_build_dir_name))
                run_cmd(ctx, 'make -j$(nproc) && make install')
        if not os.path.isdir(log4cplus_build_dir):
            ctx.fatal('Failed to build log4cplus')
        ctx.env.LOG4CPLUS_BUILD_DIR = log4cplus_build_dir
        ctx.env.INCLUDES += [os.path.join(log4cplus_build_dir, 'include')]
        ctx.env.LIBPATH += [os.path.join(log4cplus_build_dir, 'lib')]
        ctx.env.STLIB += ['log4cplus']
        ctx.msg('Log4cplus configuration:', log4cplus_build_dir)
        ctx.msg('Setup submudules', 'ok')

    def setup_common(ctx):
        ctx.env.CXXFLAGS += ['-std=c++11', '-Wextra', '-Werror', '-Wpedantic']
        ctx.env.LIB += ['pthread']
        ctx.env.INCLUDES += ['src', 'thirdparty/gtest']
        # Setup required components (submodules)
        setup_submodules(ctx)

    # Copy config to the project root
    dest_logger_cfg = os.path.join(ctx.path.abspath(), 'logger.cfg')
    if not os.path.isfile(dest_logger_cfg):
        src_cfg = os.path.join(
            ctx.path.abspath(), 'infrastructure', 'config', 'logger.cfg')
        shutil.copyfile(src_cfg,
                        dest_logger_cfg)

    # Setup debug configuration
    ctx.setenv('debug')
    ctx.load('compiler_cxx')
    setup_common(ctx)
    ctx.env.CXXFLAGS += ['-g', '-O0']
    ctx.define('DEBUG', 1)
    # Add sanitizers if required
    if ctx.options.sanitize_with:
        setup_sanitizer(ctx)
    ctx.msg('Setup debug build', 'done')
    # Setup release configuration
    ctx.setenv('release')
    ctx.load('compiler_cxx')
    setup_common(ctx)
    ctx.env.CXXFLAGS += ['-O3']
    ctx.define('NDEBUG', 1)
    ctx.msg('Setup relase build', 'done')


def post_build(dummy_ctx):
    pass


def build(ctx):
    from multiprocessing import cpu_count
    ctx.jobs = cpu_count()
    ctx.add_post_fun(post_build)
    ctx.program(source=ctx.path.ant_glob(['src/**/*.cc']),
                target='echosrv')
    ctx.program(source=ctx.path.ant_glob(['src/**/*.cc',
                                          'test/**/*.cc',
                                          'thirdparty/gtest/*.cc'],
                                         excl=['src/core/engine_launcher.cc']),
                target='testrunner',
                includes=['test'])

def init(dummy_ctx):
    for build_type in 'debug release'.split():  # pylint: disable=unused-variable
        for context_type in (BuildContext, CleanContext, InstallContext, UninstallContext):
            name = context_type.__name__.replace('Context', '').lower()
            class BuildSpecific(context_type): # pylint: disable=too-few-public-methods, unused-variable
                cmd = name + '_' + build_type
                variant = build_type

    # Prefer clang compiler over gcc
    cxx_compiler['linux'] = ['clang++', 'g++', 'icpc']

    for context_type in (BuildContext, CleanContext, InstallContext, UninstallContext):
        class BuildDefault(context_type): # pylint: disable=too-few-public-methods, unused-variable
            variant = 'debug'


def build_all(dummy_ctx):
    import waflib.Options # pylint: disable=import-error
    for item in ('build_debug', 'build_release'):
        waflib.Options.commands.insert(0, item)


def clean_all(dummy_ctx):
    import waflib.Options # pylint: disable=import-error
    for item in ('clean_debug', 'clean_release'):
        waflib.Options.commands.insert(0, item)
