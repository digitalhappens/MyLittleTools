
# MyLittleTools

MyLittleTools is a plugin for [VCV Rack](https://vcvrack.com). It
contains:

* MyLittleFavorites, favorite module handling with state saving and fast preset-loading/saving option
* MyLittleTags, fast tag-based module browsing

## MyLittleFavorites, favorite module handling

The release of VCV Rack 1.0 contains many great new features, enhancements and changes. 
There's only one features missing / lagging (I think), a good way to organize favorite modules in a patch.
In version 0.6 we had a favorite button, but this was globally, with MyLittleFavorites
you can handle up to 8 self-defined favorite modules per instance / preset.

You can have as many instances in a patch within VCV Rack as you want or use the fast preset-loading function to switch between favorite module setups. For better organization, each instance of the module can have a unique name like "BASIC" or "VCO".

## MyLittleTags, tag browsing

This is a simple module that allows fast module browsing by the tag (f.e. Filter, Attenuater or Waveshaper). It shows only tags that have modules in it. With the heart-button you can switch between the tag-list.

In future versions I will implement a hiding-feature, so you can hide specific modules if you want.

### How to use "MyLittleFavorites"

Easiest way to get started:

* Make an empty patch (Ctrl+N)
* add MyLittleFavorites
* add some modules you like to save as favorites
* on MyLittleFavorites press the "Heart-Button" (it turns red, you're in "editmode")
* now the 8 fields give you a list of currently loaded modules
* you can save one module per field, by clicking on the field from list
* now press the "Heart-Button" again (it turns black)
* now you can recall the modules by pressing the fields

Optionally, you can rename the field names in rename-mode (with the R-Button) for better reading. For example, you could save the "AudioInterface"-Module with your favorite settings and name it "ASIO".

Optionally, you can clear a field in clear-mode (with the C-Button), which empties an field by clicking on it. 

Optionally, you can fast-load a preset with the P-Button. When you have filled MyLittleFavorites with 8 of your favorite modules/states, you can save it as a preset. When you name this preset by starting with "mlf_" then this preset will appear in a list with the P-Button, you can then load it by clicking it. This way it is possible to fast switch between as many favorites as you need. The presets must be saved in the Rack/presets folder.

Each favorite is saved with it's state, which means all parameter and data settings are saved within by default. 

Optionally, you can fast-save your presets with the S-Button in the rightc corner. It will generate the filename from the name of your set and opens the file-saving dialog. You have to save this file in the preset-folder from Rack (which is done by default).

Here's a short video / short manual for MyLittleTools

[![MyLittleTools](https://img.youtube.com/vi/BCSQwT4ZiHU/100.jpg)](https://www.youtube.com/watch?v=BCSQwT4ZiHU)

## VIDEO HOW TO USE MYLITTLEFAVORITES ##

Changes in Version 1.0.2

* now the state of the modules is saved (parameters and data)
* you can rename an entry (with new R Button)
* you can clear an entry (with the C Button)
* when you save Presets in MyLittleFavorites in the global /Rack/presets-Folder starting with "mlf_"
  the presets can be recalled quickly with the P-Button (so you need only one intance of the module to store all your favorites)
* the list of currently loaded modules shows all modules currently loaded, except "MyLittleFavorites"
* presets can also quickly saved withe "S"-Button in the right corner, filename will constructed from the name of your set
* plugin.json has a donate link, you can donate anything if you want to keep up developing ;)

Changes in Version 1.0.1

* Module is wider
* the dots for module recall are now lables/fields with the name of saved the module

## Contributing

I welcome Issues and Pull Requests to this repository if you have suggestions for improvement.
