# Banditex

## Setting up

* [ ] `git clone` this repo
* [ ] [Download CMAKE](https://cmake.org/download/) if you aren't already using it
* [ ] Populate the  JUCE by running `git submodule update --init` in your repository directory
* [ ] Build n' Run: to generate an Xcode project, run `cmake -B Builds -G Xcode`. Project will be in `Build` folder which is not commited to the repo.

## Where do I put new .h / .cpp files?

New source files go in `/source`. All `.h` and `.cpp` files in that directory will be available to include in your plugin target and your tests. 

Tests go in `/tests`. Just add .cpp files there and they will be available in the Tests target.

I recommend not stuffing everything into the boilerplate PluginEditor/PluginProcessor files. Sure, go ahead make a mess at first. But then clean them up and just include your source from there.

## How do I add another module?

Additional 3rd party JUCE modules go in `/modules`. You can add third party git submodules there (like the inspector is set up). Remember to not only call `juce_add_module` but add it to the `target_link_libraries` list!

I (and others, including some of the JUCE team) recommend moving as much as your application code into modules as possible. For example, if you tend to roll your own widgets, pop those into a module, you'll thank yourself later.

A few reasons to do so:

* Re-usability. You can use modules across projects.
* Testability. You can test modules in isolation from each other. When sticking test in modules, it's common to guard `.cpp` files with something like `#ifdef RUN_MY_TESTS` and set via `target_compile_definitions` in `Tests` target.
* Sanity. You can keep the root project tidy, focused on the application logic.
* Compile-friendliness. Each JUCE module is its own compilation unit. If you change a file in a module, only that one module needs to rebuild. It also means you can work on *only* the module in a separate CMake project, which is a very nice/fast life.

Don't worry about all of this if you are new to JUCE. Just keep it in mind as you grow.

## What's the deal with BinaryData?

Your binary data CMake target is called `Assets`.

You need to include `BinaryData.h` to access it. 

> [!IMPORTANT]
> You may have to configure the project (just hit build in your IDE) to build juceaide before the header will be available.

## What's the deal with code signing and notarization?

This repo code signs Windows via Azure Key Vault. [Read more about how to set it up blog](https://melatonin.dev/blog/how-to-code-sign-windows-installers-with-an-ev-cert-on-github-actions/).

It also code signs and notarizes on macOS. Again, you can [read my article for details](https://melatonin.dev/blog/how-to-code-sign-and-notarize-macos-audio-plugins-in-ci/).

## How do I update my Pamplejuce-based project?

1. Update with the latest CMake version [listed here](https://github.com/lukka/get-cmake), or the latest version supported by your toolchain like VS or Clion.
2. Update JUCE with `git submodule update --remote --merge JUCE`
3. Update the inspector with `git submodule update --remote --merge modules/melatonin_inspector`
4. Check for an [IPP update from Intel](https://github.com/oneapi-src/oneapi-ci/blob/master/.github/workflows/build_all.yml#L10).
5. If you want to update to the latest CMake config Pamplejuce uses, first check the repository's [CHANGELOG](https://github.com/sudara/cmake-includes/blob/main/CHANGELOG.md) to make sure you are informed of any breaking changes. Then. `git submodule update --remote --merge cmake`. Unfortunately, you'll have to manually compare `CMakeLists.txt`, but it should be pretty easy to see what changed.

## How to Cut A GitHub Release

Cut a release with downloadable assets by creating a tag starting with `v` and pushing it to GitHub. Note that you currently *must push the tag along with an actual commit*.

I recommend the workflow of bumping the VERSION file and then pushing that as a release, like so:

```
# edit VERSION
git commit -m "Releasing v0.0.2"
git tag v0.0.2
git push --tags
```

> [!IMPORTANT]
> Releases are set to `prerelease`! This means that uploaded release assets are visible to other users (on public repositories), but not explicitly listed as the latest release until you "publish" in the GitHub UI. 

## I'm new to GitHub Actions, what do I need to know?

CI will run against latest Linux, Windows, and macOS unless modified. You can do it all for free on public repos.

> [!NOTE]
> You can push a commit with `[ci skip]` in the message if you are just doing things like updating the README or checking in a WIP that you know will fail CI.

## How do variables work in GitHub Actions?

It can be confusing. The documentation is a big fragmented. Here are some tips.

1. Things in double curly braces like `${{ matrix.name }}` are called ["contexts or expressions"](https://docs.github.com/en/free-pro-team@latest/actions/reference/context-and-expression-syntax-for-github-actions) and can be used to get, set, or perform simple operations.
2. In "if" conditions you can omit the double curly braces, as the whole condition is evaluated as an expression: `if: contains(github.ref, 'tags/v')`
3. You can set variables for the whole workflow to use in ["env"](https://docs.github.com/en/free-pro-team@latest/actions/reference/workflow-syntax-for-github-actions#env)
4. Reading those variables is done with the [env context](https://docs.github.com/en/free-pro-team@latest/actions/reference/context-and-expression-syntax-for-github-actions#env-context) when you are inside a `with`, `name`, or `if`: `${{ env.SOME_VARIABLE }}`
5. Inside of `run`, you have access to bash ENV variables *in addition* to contexts/expressions. That means `$SOME_VARIABLE` or `${SOME_VARIABLE}` will work but *only when using bash* and [not while using powershell on windows](https://docs.github.com/en/free-pro-team@latest/actions/reference/workflow-syntax-for-github-actions#using-a-specific-shell). The version with curly braces (variable expansion) is often used [when the variable is forming part of a larger string to avoid ambiguity](https://stackoverflow.com/questions/8748831/when-do-we-need-curly-braces-around-shell-variables). Be sure that the ENV variable was set properly in the workflow/job/step before you use it. And if you need the variable to be os-agnostic, use the env context.

## What's up with the `SharedCode` interface target, is it needed?

If you want to build both plugin targets and a test target, unfortunately the additional abstraction of the INTERFACE `SharedCode` target is needed  (as of Nov 2023). If you aren't running tests, shame on you, but hey, you can simply edit the CMake and get rid of it :)

The summary: JUCE modules build separately for each target. You need to link against them with PRIVATE visibility.  But both JUCE's internal plugin shared code target (which powers the formats like AU, VST, etc) and Pamplejuce's `Tests` target need to link against the same JUCE modules. 

This becomes a problem when you link `Tests` to `YourPlugin` target, as it causes ODL issues and confuses your IDE. Additionally, it is hard/impossible to set different compile definitions for the `Tests` target vs. plugin targets (for example, you'll probably need to enable the deprecated modal loops, guard macros for running tests, etc).

I spoke with [Reuben at JUCE a bit about this here](https://forum.juce.com/t/windows-linker-issue-on-develop/55524/2) and there's a Pamplejuce [issue with background here](https://github.com/sudara/pamplejuce/issues/31). 

## What if I need to include files not in modules and not in `/source`?

If you have control over the files, I highly recommend taking 3 minutes to make a JUCE module — if nothing else than to wrap the code you need and make the build system nice and easy. See the [module API](https://github.com/juce-framework/JUCE/blob/master/docs/JUCE%20Module%20Format.md), or other JUCE modules for an example on how to do it. 

If that's not an option, you could add more directories in the `file(GLOB_RECURSE SourceFiles` line in the `CMakeLists.txt` and maybe fiddle with `source_group` to have things show up in your IDE. But again, I recommend sticking with JUCE modules and keeping the IDE source tree reflective of your filesystem.

## How do I build JuceHeader.h

Using `JuceHeader.h` has been deprecated for some time — if it's a new project, definitely avoid it! 

Instead, directly include the `.h` files you need from the juce modules you are using, like `#include "juce_gui_basics/juce_gui_basics.h"`

If you are converting an older project, it's still worth the conversion away from `JuceHeader.h` to using the actual juce modules you need. You'll get faster compilation, autocomplete, etc. You can [see an example of the conversion I did for the pluginval project](https://github.com/Tracktion/pluginval/pull/90/files). It's less scary than you think: just make sure the `juce::` prefix is added everywhere, try to compile and your IDE will yell at you when you need to include one of the modules :)

## Original References & Inspiration

### CMake

* [The "Modern CMake" gitbook](https://cliutils.gitlab.io/) which also has a section on [https://cliutils.gitlab.io/modern-cmake/chapters/testing/catch.html](Catch2).
* [Effective Modern CMake](https://gist.github.com/mbinna/c61dbb39bca0e4fb7d1f73b0d66a4fd1)
* JUCE's announcement of [native CMake support](https://forum.juce.com/t/native-built-in-cmake-support-in-juce/38700)
* [Eyalamir Music's JUCE / CMake prototype repository](https://github.com/eyalamirmusic/JUCECmakeRepoPrototype)

### GitHub Actions

* [Christian Adam's HelloWorld CMake and ccache repo](https://github.com/cristianadam/HelloWorld)
* [Maxwell Pollack's JUCE CMake + GitHub Actions repo](https://github.com/maxwellpollack/juce-plugin-ci)
* [Oli Larkin's PDSynth iPlug2 template](https://github.com/olilarkin/PDSynth)
* [Running pluginval in CI](https://github.com/Tracktion/pluginval/blob/develop/docs/Adding%20pluginval%20to%20CI.md)

### Catch2 & CTest

* [Catch2's docs on CMake integration](https://github.com/catchorg/Catch2/blob/devel/docs/cmake-integration.md)
* [Roman Golyshev's Github Actions & Catch2 repo](https://github.com/fedochet/github-actions-cpp-test)
* [Matt Clarkson's CMakeCatch2 repo](https://github.com/MattClarkson/CMakeCatch2)
* [CMake Cookbook example](https://github.com/dev-cafe/cmake-cookbook/tree/master/chapter-04/recipe-02/cxx-example)
* [Unit Testing With CTest](https://bertvandenbroucke.netlify.app/2019/12/12/unit-testing-with-ctest/)
* [Mark's Catch2 examples from his 2020 ADC talk](https://github.com/Sinecure-Audio/TestsTalk)

### Packaging, Code Signing and Notarization 

* [iPlug Packages and Inno Setup scripts](https://github.com/olilarkin/wdl-ol/tree/master/IPlugExamples/IPlugEffect/installer)
* [Surge's pkgbuild installer script](https://github.com/kurasu/surge/blob/master/installer_mac/make_installer.sh)
* [Chris Randall's PackageBuilder script](https://forum.juce.com/t/vst-installer/16654/15)
* [David Cramer's GA Workflow for signing and notarizing](https://medium.com/better-programming/indie-mac-app-devops-with-github-actions-b16764a3ebe7) and his [notarize-cli tool](https://github.com/bacongravy/notarize-cli)
