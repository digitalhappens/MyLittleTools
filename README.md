
# MyLittleTools

MyLittleTools is a plugin for [VCV Rack](https://vcvrack.com). It
contains (at the moment) one module:

* MyLittleFavorites, favorite module handling

## MyLittleFavorites, favorite module handling

The release of VCV Rack 1.0 contains many great new features, enhancements and changes. 
There's only one features missing (I think), a good way to organize favorite modules in a patch.
In version 0.6 we had a favorite button, but this was globally, with MyLittleFavorites
you can handle up to 8 self-defined favorite modules per instance.

You can have as many instances in a patch within VCV Rack as you want. For better organization,
each instance of the module can have a unique name like "BASIC" or "VCO".

### How to use

Easiest way to get started:

* Make an empty patch (Ctrl+N)
* add MyLittleFavorites
* add some modules you like to save as favorites
* on MyLittleFavorites press the "Heart-Button" (it turns red, you're in "editmode")
* now the 8 fields give you a list of currently loaded modules
* you can save one module per field, by clicking on the field from list
* now press the "Heart-Button" again (it turns black)
* now you can recall the modules by pressing the fields

Changes in Version 1.0.1

* Module is wider
* the dots for module recall are now lables/fields with the name of saved the module

Each instance of MyLittleFavorites can hold different favorites and is saved within the patch or autosave-patch of vcv rack.
You can save presets, to recall your favorites in other patches.





## Contributing

I welcome Issues and Pull Requests to this repository if you have
suggestions for improvement. 

This plugin is released under the totally free Public License.
