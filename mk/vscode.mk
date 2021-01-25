# Support for debugging using Microsoft Visual Studio Code, along with OpenOCD.
# VSCode puts installed extensions into ~/.vscode/
# We are creating our launch config in $(CURDIR)/.vscode/launch.json
#
# to start clean, you can:
# > rm -rf ~/.vscode/
# > rm -rf .vscode/

vscode_instructions:
	@echo Instructions and config file based on
	@echo https://lonesometraveler.github.io/2020/03/27/debug.html
	@echo https://www.reddit.com/r/embedded/comments/7q1hri/visual_studio_code_extension_for_debugging_arm/
	@echo https://devzone.nordicsemi.com/f/nordic-q-a/53118/visual-studio-code-development-with-gcc-and-openocd
	@echo Inside VSC, install Cortex-Debug extension authored by marus25:
	@echo "    <ctrl><shift>D to open Debug view"
	@echo "    click 'create a launch a debug file'"
	@echo "    click 'More...' in dropdown list for 'Select Environment'"
	@echo "    start typing ' Cortex' (with space after prepopulated 'tag:debuggers @sort:installs'"
	@echo "    click 'Install' for 'Cortex-Debug' by marus25"
	@echo "    "
	@echo "    Once it's installed, 'x' the window for it, and <ctrl><shift>D again!"
	@echo "    click 'create a launch and debug file AGAIN!"
	@echo "    this time you'll see 'Cortex-Debug' in the dropdown list!"


.vscode/launch.json:
	@mkdir -p .vscode
	@echo "$$VSC_LAUNCH_CFG" > $@

# debugging rarely works.  maybe first time after a replug of USB cable,
# or maybe also relates to if settings.json exists?
.vscode/settings.json:
	@mkdir -p .vscode
	#@echo "$$VSC_DEFAULT_SETTINGS" > $@

debug.vscode: vscode_instructions | .vscode/launch.json .vscode/settings.json
	PATH=$(PATH):$(abspath gcc-arm-none-eabi/bin/) && code

# Here's the contents of the launch.json we'll be creating.
# Weird syntax of "define" creates a multi-line string,
# then we use "export" to put it in env, then we echo it
# with $$().  Otherwise it's hard to echo a multi-line string
# from Make in a shell command.

# Notes on settings:
# "request": can be "launch" or "attach".  Can make two config files to allow both options from VSCode GUI.
# "svdFile": if no registers for peripherals show up in debugger, could try using  this
# "runToMain": true, if "request": "launch", else not allowed to be present.
define VSC_LAUNCH_CFG
{
    "version": "0.2.0",
    "configurations": [
        {
            "type": "cortex-debug",
            "request": "launch",
            "name": "Debug (OpenOCD)",
            "servertype": "openocd",
            "cwd": "${workspaceRoot}",
            "preLaunchTask": "",
            "runToMain": true,
            "executable": "$(abspath $(TARGET))",
            "device": "atsame70q21",
            "configFiles": [
                "/usr/share/openocd/scripts/board/atmel_same70_xplained.cfg"
            ],
            "swoConfig": {
                "enabled": true,
                "cpuFrequency": 300000000,
                "swoFrequency": 1800000,
                "source": "probe",
                "decoders": [
                    { "type": "console", "label": "ITM", "port": 0 }
                ]
            }
        }
    ]
}
endef

export VSC_LAUNCH_CFG

define VSC_DEFAULT_SETTINGS
{
"files.watcherExclude": {
    "**/mk/**": true,
    "**/obj/**": true,
    "**/gcc-arm-none-eabi/**": true,
    "**/eclipse/**": true
}
}
endef

export VSC_DEFAULT_SETTINGS