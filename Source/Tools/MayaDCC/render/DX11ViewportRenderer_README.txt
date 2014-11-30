
DX11ViewportRenderer Viewport Renderer Plugin
============================================
Last updated: September 14, 2010

This plugin is a simple example of how Direct3D can be used as the renderer for a 
interactive modeling viewport. It is not meant to be a full replacement for 
existing viewport renderers.

The plugin is only an example, and is provided as is to demonstrate the interfaces 
required to be overridden for a MViewportRenderer subclass. MViewportRenderer
is a class introduced with Maya version 8.

This plug-in is a partial port of the D3DViewportRenderer plug-in to DirectX 11
(D3DViewportRenderer uses DirectX 9). Not all of the functionality of that plug-in
is implemented currently. In particular, the post effects are completely absent.

Building the plugin:
--------------------

The provided solution files have been tested for building the 32-bit version 
and 64-bit version of the  plugin using Microsoft VC9 (2008). The DirectX Developer SDK 
compiled with is the June 2010 release from Microsoft. The include and link library directories will 
need to be modified to point to the location that Maya's SDK has been installed.

The preprocessor directive DX11_SUPPORTED must be set to 
have the plugin use Direct3D code, otherwise this plugin will basically 
do nothing. The solution files have this directive set.

Using the plugin:
-----------------
1. Installing the plugin:

Copy the plugin to somewhere in your plugin path. e.g. (bin/plug-ins directory)
If the SDK has not been installed then the DirectX End-User Runtimes (June 2010) must be 
installed as the plugin is dependent on these libraries. 

2. Installing effects files: (bin directory)
Copy the .fx and .hlsl files to a folder named 'HLSL' under the Maya 'bin' folder.

3. Installing MEL UI:
Copy the mel files to the scripts/others directory. Replace any existing script of 
the same name.

4. Loading the plugin:

Load the plugin from the Plugin Manager window. Should the plugin fail to load, recheck that 
the correct dependent DLLs have been installed.

Once the plugin is registered, a new "DX11 Renderer" menu item should appear 
under the "Renderer" menu in each of  the 3d modeling viewports. Selecting that menu item 
will invoke usage of the renderer. 

An option box has also been written to provide an interface to effects options on the 
renderer. Should the options have no effect, check that the .fx files have been installed 
properly.

Feedback:
---------
If you plan to make any additions or changes to the plugin, or have any feedback, please send 
this to us so that we can incorporate improvements into the plugin. Thank you.

---









	

