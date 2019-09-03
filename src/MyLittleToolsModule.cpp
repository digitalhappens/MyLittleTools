#include "MyLittleTools.hpp"
#include "plugin.hpp"
#include "window.hpp"
#include "tag.hpp"
#include <algorithm>
#include <functional>
#include <random>
#include <libgen.h> // for dirname and basename
#include <ui/Tooltip.hpp>
#include <osdialog.h>



static const char PRESET_FILTERS[] = "VCV Rack module preset (.vcvm):vcvm";

static inline std::string trim(std::string str)
{
    str.erase(0, str.find_first_not_of(' '));       //prefixing spaces
    str.erase(str.find_last_not_of(' ')+1);         //surfixing spaces
    return str;
}


static const char* convertAndCombine(std::string input1, int input2)
{
  char* buffer = new char[256];
  sprintf (buffer, "%s%d",input1.c_str(), input2);
  return buffer;
}

static bool isForbidden( char c )
{
    static std::string forbiddenChars( "\\/:?\"<>|*&" );
    return std::string::npos != forbiddenChars.find( c );
}



struct MyLittleTools : Module {
  enum ParamIds {
    NUM_PARAMS
    
  };
  enum InputIds {
    MODULE_1,
    NUM_INPUTS
  };
  enum OutputIds {
    NUM_OUTPUTS
  };
  enum LightIds {
    NUM_LIGHTS
  };

  std::string* _plugin;
  std::string* _module;
  std::string* _name;
  json_t *textFieldText;

  json_t *_jsondata[8];
  json_t *_json[8];

  int _tagsListJump;

  int _slot;
  bool _editMode;
  bool _resetMode;
  bool _renameMode;
  bool _bTagsListed;
  bool _allLabelReset;
  bool _savePreset;
  bool setTextFieldText;
  std::string tags[100];
  int tagsCount;

  MyLittleTools() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    _plugin = new std::string[8];
    _module = new std::string[8];
    _name = new std::string[8];

    _editMode = false;
    _resetMode = false;
    _renameMode = false;
    _tagsListJump = 0;
    _bTagsListed = false;
    _allLabelReset = false;
    _savePreset = false;
    setTextFieldText = false;
  }

 void RaiseModel(std::string plugin, std::string module)
 {
    Model *model = rack::plugin::getPlugin(plugin)->getModel(module);
    if (model)
    {
      ModuleWidget *moduleWidget = model->createModuleWidget();
      if (!moduleWidget) {
        return;
      }
      APP->scene->rack->addModuleAtMouse(moduleWidget);
    
      // recall paramters and data
      json_t *jt = moduleWidget->module->toJson();
      json_object_set_new(jt, "params", _json[_slot]);
      json_object_set_new(jt, "data", _jsondata[_slot]);
      moduleWidget->module->fromJson(json_deep_copy(jt));

      history::ModuleAdd *h = new history::ModuleAdd;
      h->name = "create module";
      h->setModule(moduleWidget);
      APP->history->push(h);
    }
  }

void RaiseModelSimple(std::string plugin, std::string module)
{
    Model *model = rack::plugin::getPlugin(plugin)->getModel(module);
    if (model)
    {
      ModuleWidget *moduleWidget = model->createModuleWidget();
      if (!moduleWidget) {
        return;
      }
      APP->scene->rack->addModuleAtMouse(moduleWidget);

      history::ModuleAdd *h = new history::ModuleAdd;
      h->name = "create module";
      h->setModule(moduleWidget);
      APP->history->push(h);
    }
  }

  void listTags()
  {
    bool tagFounded = false;
    tagsCount = 0;

      for (int i = 0; i < (int)tag::tagAliases.size(); i++) {
      //for (const std::string& tag : tag::tagAliases) {
        
        // choose the smallest tag in size!!!!!
        size_t iamshort = 5000;
        int chid = 0;
        int counter = 0;

        for (const std::string& alias : tag::tagAliases[i]) {
          
          if (alias.length() < iamshort){
            chid = counter;
            iamshort = alias.length();
          }
          counter++;
        }

        std::string tag = tag::tagAliases[i][chid];


      for (const auto& p : rack::plugin::plugins) 
      {
        for (const auto& m : p->models) 
        {

          if (tagFounded)
          {
            tagFounded = false;
            break;
          }

          
          for (const auto& t : m->tags) 
          {
            if (i == t) 
            {
              INFO("TAG %s", tag.c_str());
              for (int i = 0; i < tagsCount; i++)
              {
                if (tags[i] == tag)
                {
                  tagFounded = true;
                  break;
                }
              }

              if (!tagFounded)
              {
                tags[tagsCount] = tag;
                tagsCount++;
                tagFounded = true;

                break;
              }
            }
          }
        }
      }
    }
  }


  void setEditMode(bool value)
  {
    _editMode = value;
  }

  bool getEditMode()
  {
    return _editMode;
  }

  std::string getSavedModule(int index)
  {
    return _module[index];
  }

  std::string getSavedName(int index)
  {
    return _name[index];
  }

  std::string getSavedPlugin(int index)
  {
    return _plugin[index];
  }

  void setSavedModule(int index, std::string module)
  {
    _module[index] = module;
  }

  void setSavedName(int index, std::string name)
  {
      _name[index] = name;
  }

  void setSlot(int slot)
  {
    _slot = slot;
  }

  void setSavedPlugin(int index, std::string plugin)
  {
    _plugin[index] = plugin;
  }

  void onReset() override {
    _allLabelReset = true;
    for (int i = 0; i < 8; i++) {
      setSavedPlugin(i, "");
      setSavedModule(i, "");
      setSavedName(i, "");
      _json[i] = NULL;
      _jsondata[i] = NULL;
    }
    textFieldText = NULL;
  }

  void simpleReset(int i) {
      setSavedPlugin(i, "");
      setSavedModule(i, "");
      setSavedName(i, "");
      _json[i] = NULL;
      _jsondata[i] = NULL;
  }

  json_t *dataToJson() override {
      json_t *rootJ = json_object();

      for (int i = 0; i < 8; i++)
      {
        if (_plugin[i] != "" && _module[i] != "") 
        {
          json_object_set_new(rootJ, convertAndCombine("plugin", i), json_string(_plugin[i].c_str()));
          json_object_set_new(rootJ, convertAndCombine("module", i), json_string(_module[i].c_str()));
          json_object_set_new(rootJ, convertAndCombine("name", i), json_string(_name[i].c_str()));

          if (_json[i])
          {
            json_object_set_new(rootJ, convertAndCombine("module_settings_json", i), json_deep_copy(_json[i]));
          }
          
          if (_jsondata[i])
          {
            json_object_set_new(rootJ, convertAndCombine("module_settings_jsondata", i), json_deep_copy(_jsondata[i]));
          }
        }
      }

      return rootJ;    
   }

  void dataFromJson(json_t *rootJ) override {
    //ModuleWidget::fromJson(rootJ);

    for (int i = 0; i < 8; i++)
    {
      json_t *plug = json_object_get(rootJ, convertAndCombine("plugin", i));
      json_t *mod = json_object_get(rootJ, convertAndCombine("module", i)); 
      json_t *name = json_object_get(rootJ, convertAndCombine("name", i));  

      if (plug)
      {
        _plugin[i] = json_string_value(plug);
      }

      if (mod)
        _module[i] = json_string_value(mod);

      if (name)
        _name[i] = json_string_value(name);
      else
        _name[i] = "";


      _json[i] = json_deep_copy(json_object_get(rootJ, convertAndCombine("module_settings_json", i)));
      _jsondata[i] = json_deep_copy(json_object_get(rootJ, convertAndCombine("module_settings_jsondata", i)));

    }
  }

  void process(const ProcessArgs &args) override {

  }
};

struct ModuleMenuItem : ui::MenuItem {
    MyLittleTools *module;
    int slot;
    std::string pluginName;
    std::string moduleName;
    std::string mname;
    std::string IAM;
    json_t *jsondata;
    json_t *json;

    void setModule(MyLittleTools *module) {
      this->module = module;
    }

    void onAction(const event::Action &e) override 
    {
      if (IAM == "MyLittleFavorites")
      {
        module->setSavedModule(slot, moduleName);
        module->setSavedName(slot, mname);
        module->setSavedPlugin(slot, pluginName);
        module->_json[slot] = json_deep_copy(json);
        module->_jsondata[slot] = json_deep_copy(jsondata);
      }

      if (IAM == "MyLittleTags")
      {
        module->RaiseModelSimple(pluginName, moduleName);
      }
    }
};


struct PresetBrowserMenuItem : ui::MenuItem {
    MyLittleTools *module;

    std::string IAM;
    std::string presetName;

    void setModule(MyLittleTools *module) {
      this->module = module;
    }

    void onAction(const event::Action &e) override 
    {
        ModuleWidget *moduleWidget = APP->scene->rack->getModule(module->id);
        //module->onReset();
        moduleWidget->resetAction();
        moduleWidget->loadAction(asset::user("presets") + "/" + presetName);
    }

    void loadAction(std::string filename) {
      INFO("Loading preset %s", filename.c_str());

      FILE *file = fopen(filename.c_str(), "r");
      if (!file) {
        WARN("Could not load patch file %s", filename.c_str());
        return;
      }
      DEFER({
        fclose(file);
      });

      json_error_t error;
      json_t *moduleJ = json_loadf(file, 0, &error);
      if (!moduleJ) {
        std::string message = string::f("File is not a valid patch file. JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
        //osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, message.c_str());
        return;
      }
      DEFER({
        json_decref(moduleJ);
      });

    
      module->fromJson(moduleJ);

    }


};

struct presetButton : SvgButton {
  MyLittleTools *module;
  std::shared_ptr<Svg> svg1;
  std::shared_ptr<Svg> svg2;

  presetButton() {
    svg1 = APP->window->loadSvg(asset::plugin(pluginInstance, "res/preset1.svg"));
    svg2 = APP->window->loadSvg(asset::plugin(pluginInstance, "res/preset2.svg"));
    addFrame(svg1);
    addFrame(svg2);
  }

  void setModule(MyLittleTools *module) {
    this->module = module;
  }

  virtual void onAction(const event::Action &e) override {
    module->_renameMode = false;
    module->_resetMode = false;
    module->_editMode = false;
    createContextMenu();
  }

  void createContextMenu() {
    ui::Menu *menu = createMenu();
    menu->addChild(createMenuLabel("load a preset (presets starting with mlf_*.vcvm)"));

    std::string path = asset::user("presets");
    for (const std::string &presetPath : system::getEntries(path)) {
      char *pathDup = strdup(presetPath.c_str());
      std::string fn = basename(pathDup);
      if (fn.rfind("mlf_",0) == 0)
      {
        PresetBrowserMenuItem *item1 = new PresetBrowserMenuItem;
        item1->IAM = "MyLittleFavorites";
        item1->setModule(module);
        item1->text = string::filename(presetPath);
        item1->presetName = string::filename(presetPath);
        menu->addChild(item1);
      }
    }
  }
};

struct clearButton : SvgButton {
  MyLittleTools *module;
  std::shared_ptr<Svg> svg1;
  std::shared_ptr<Svg> svg2;
  int buttonid;
  bool bset;

  clearButton() {
    svg1 = APP->window->loadSvg(asset::plugin(pluginInstance, "res/reset1.svg"));
    svg2 = APP->window->loadSvg(asset::plugin(pluginInstance, "res/reset2.svg"));
    addFrame(svg1);
    bset = false;
  }

  void setModule(MyLittleTools *module) {
    this->module = module;
  }

  void step() override {
    if (bset)
    {
      if (!module->_resetMode)
      {
        fb->dirty = true;
        frames[0] = svg1;
        sw->setSvg(frames[0]);
        bset = false;
      }      
    }
    Widget::step();
  }

  virtual void onAction(const event::Action &e) override {
    if (module->_resetMode)
    {
      module->_resetMode = false;
      frames[0] = svg1;
      module->_editMode = false;
      module->_renameMode = false;
      bset = true;
    }
    else
    {
      module->_resetMode = true;
      frames[0] = svg2;
      module->_editMode = false;
      module->_renameMode = false;
      bset = true;
    }
  }
};

struct renameButton : SvgButton {
  MyLittleTools *module;
  std::shared_ptr<Svg> svg1;
  std::shared_ptr<Svg> svg2;
  bool bset;
  int buttonid;

  renameButton() {
    svg1 = APP->window->loadSvg(asset::plugin(pluginInstance, "res/rename1.svg"));
    svg2 = APP->window->loadSvg(asset::plugin(pluginInstance, "res/rename2.svg"));
    bset = false;
    addFrame(svg1);
  }

  void setModule(MyLittleTools *module) {
    this->module = module;
  }

  void step() override {
    if (bset)
    {
      if (!module->_renameMode)
      {
        fb->dirty = true;
        frames[0] = svg1;
        sw->setSvg(frames[0]);
        bset = false;
      }      
    }
    Widget::step();
  }

  virtual void onAction(const event::Action &e) override {
    if (module->_renameMode)
    {
      module->_renameMode = false;
      frames[0] = svg1;
      module->_editMode = false;
      module->_resetMode = false;
      bset = true;
    }
    else
    {
      module->_renameMode = true;
      frames[0] = svg2;
      module->_editMode = false;
      module->_resetMode = false;
      bset = true;
    }
  }
};

struct heartButton : SvgButton {
    MyLittleTools *module;

    std::shared_ptr<Svg> svg1;
    std::shared_ptr<Svg> svg2;

    ui::Label *labelEditMode;
    std::string IAM;

    std::vector<std::string> presetPaths;

    bool bset;

    heartButton() {
      svg1 = APP->window->loadSvg(asset::plugin(pluginInstance, "res/heart1.svg"));
      svg2 = APP->window->loadSvg(asset::plugin(pluginInstance, "res/heart2.svg"));

      addFrame(svg1);
      addFrame(svg2);
      bset = false;

    labelEditMode = new ui::Label;
    // brandLabel->fontSize = 16;
    labelEditMode->box.pos.x = 22;
    labelEditMode->box.pos.y = 0;
    labelEditMode->color = nvgRGB(0x30, 0x30, 0x30);
    //labelEditMode->text = "edit mode";
    addChild(labelEditMode);
    viewEditMode(false);

    }

  void step() override {
    if (bset)
    {
      if (!module->_editMode)
      {
        fb->dirty = true;
        frames[0] = svg1;
        sw->setSvg(frames[0]);
        bset = false;
      }      
    }
    Widget::step();
  }

  void setModule(MyLittleTools *module) {
    this->module = module;
  }

  void viewEditMode(bool mode)
  {
    if (IAM == "MyLittleFavorites")
    {
      /*if (mode)
        labelEditMode->text = "edit mode";
      else
        labelEditMode->text = "<- edit";
        */
    }

    if (IAM == "MyLittleTags")
    {
      labelEditMode->text = "TAGS";
    }
  }

  virtual void onAction(const event::Action &e) override {
    //void onDragStart(const event::DragStart &e) override {
    
    if (IAM == "MyLittleFavorites")
    {
        if (module->getEditMode())
        {
          module->setEditMode(false);
          frames[0] = svg1;
          viewEditMode(false);
          module->_renameMode = false;
          module->_resetMode = false;
          bset = true;
        }
        else
        {
          module->setEditMode(true);
          frames[0] = svg2;
          viewEditMode(true);
          module->_renameMode = false;
          module->_resetMode = false;
          bset = true;
        }
    }

    if (IAM == "MyLittleTags")
    {
      if (!module->_bTagsListed)
      {
        module->listTags();
        module->_bTagsListed = true;
      }
      else
      {
        module->_tagsListJump += 8;
      }
    }

  }
};


struct RenameField : ui::TextField {
  MyLittleTools *module;
  int buttonid;

  void setModule(MyLittleTools *module) {
    this->module = module;
  }

  void step() override {
    // Keep selected
    APP->event->setSelected(this);
    TextField::step();
  }

  void onSelectKey(const event::SelectKey &e) override {
    if (e.action == GLFW_PRESS && (e.key == GLFW_KEY_ENTER || e.key == GLFW_KEY_KP_ENTER)) 
    {
      if (module->_name[buttonid] != module->_module[buttonid])
        module->setSavedName(buttonid, text);

      ui::MenuOverlay *overlay = getAncestorOfType<ui::MenuOverlay>();
      overlay->requestDelete();
      e.consume(this);
    }

    if (e.action == GLFW_PRESS && (e.key == GLFW_KEY_ESCAPE)) 
    {
      ui::MenuOverlay *overlay = getAncestorOfType<ui::MenuOverlay>();
      overlay->requestDelete();
      e.consume(this);
    }

    if (!e.getTarget())
      TextField::onSelectKey(e);
  }
};


struct screwPlus : SvgButton {
  MyLittleTools *module;
  screwPlus() {
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/screw1.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/screw2.svg")));

  }

  void setModule(MyLittleTools *module) {
    this->module = module;
  }


  void onAction(const event::Action &e) override {
    if (module)
    {
      module->_savePreset = true;
      //ModuleWidget *moduleWidget = APP->scene->rack->getModule(module->id);
      //moduleWidget->saveDialogX();
    }
  }
};



struct slotButton : SvgButton {
  MyLittleTools *module;
  int buttonid;
  ui::Label *labelName;
  std::string IAM;
  RenameField *renameField;

  slotButton() {
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/sb0.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/sb1.svg")));

    labelName = new ui::Label;
    labelName->box.pos.x = 5;
    labelName->box.pos.y = 2;
    labelName->color = nvgRGB(0x10, 0x10, 0x10);
    
    addChild(labelName);
  }


  void step() override {
    if (IAM == "MyLittleFavorites")
    {
      if (module)
        setLabelName();
    }

    if (IAM == "MyLittleTags")
    {
      if (module)
      {
        if (!module->_bTagsListed)
        {
          module->listTags();
          module->_bTagsListed = true;
        }
        
        if (module->_bTagsListed)
        {
          if (buttonid == 0 && module->tags[module->_tagsListJump] == "")
            module->_tagsListJump = 0;
          else
            setLabelNameTag(module->tags[module->_tagsListJump + buttonid]);
        }
      }
    }

    Widget::step();
  }


  void setModule(MyLittleTools *module) {
    this->module = module;
  }

  void setLabelName()
  {
    if (module)
    {
      std::string str = module->getSavedModule(buttonid);
      std::string strname = module->getSavedName(buttonid);

      if (strname != "")
        str = strname;

      if (str != "")
      {
        unsigned sz = str.size();
        if (sz > 15)
        {
          str.resize(15);
          str = str + "...";
        }

        if (labelName->text != str)
          labelName->text = str;
      }
    }
  }

  void setLabelNameTag(std::string name)
  {
    labelName->text = name;
  }

  virtual void onAction(const event::Action &e) override {
    if (IAM == "MyLittleFavorites")
    {
      module->_slot = buttonid;
      
      if (module->_resetMode)
      {
        // Einzelreset
        module->simpleReset(buttonid);
        labelName->text = "";
        return;
      }

      if (module->_renameMode)
      {
        makeContextRenameOnButton(buttonid);
        return;
      }
      else
      {
        if (module->getEditMode())
          makeContextMenuOnButton(buttonid);
        else
        {
          if (module->getSavedPlugin(buttonid) != "")
          {
            labelName->text = module->getSavedModule(buttonid);
            module->RaiseModel(module->getSavedPlugin(buttonid), module->getSavedModule(buttonid));
          }
        }
      }
    }

    if (IAM == "MyLittleTags")
    {
      if (labelName->text != "")
        makeContextMenuOnButton_tag(buttonid);
    }


  }

  void makeContextRenameOnButton(int slot)
  {
    if (labelName->text != "")
    {
      ui::Menu *menu = createMenu();
      menu->addChild(createMenuLabel("rename entry"));

      renameField = new RenameField;
      renameField->buttonid = buttonid;
      renameField->module = module;
      renameField->text = labelName->text;
      renameField->box.size.x = 120;
      menu->addChild(renameField);
      APP->event->setSelected(renameField);
    }
  }

  void makeContextMenuOnButton(int slot)
  {
    ui::Menu *menu = createMenu();
    menu->addChild(createMenuLabel("currently loaded Rack modules"));

    for (widget::Widget *w : APP->scene->rack->moduleContainer->children) {
      ModuleWidget *mw = dynamic_cast<ModuleWidget*>(w);
      
      json_t *moduleJ = mw->toJson();
      json_t *pluginSlugJ = json_object_get(moduleJ, "plugin");
      json_t *modelSlugJ = json_object_get(moduleJ, "model");


      std::string mn = json_string_value(modelSlugJ);

      if (mn != "MyLittleFavorites")
      {
        ModuleMenuItem *item1 = new ModuleMenuItem;
        item1->slot = slot;
        item1->IAM = "MyLittleFavorites";
        item1->setModule(module);

        item1->json = new json_t();
        item1->json = json_deep_copy(json_object_get(mw->module->toJson(), "params"));

        item1->jsondata = new json_t();
        item1->jsondata = json_deep_copy(json_object_get(mw->module->toJson(),"data"));
        
        item1->text = json_string_value(pluginSlugJ);
        item1->rightText = json_string_value(modelSlugJ);
        item1->pluginName = json_string_value(pluginSlugJ);
        item1->moduleName = json_string_value(modelSlugJ);
        menu->addChild(item1);
      }
    } 
  }

  void makeContextMenuOnButton_tag(int slot)
  {
    ui::Menu *menu = createMenu();
    menu->addChild(createMenuLabel("in this category"));

    for (const auto& p : rack::plugin::plugins) 
    {
      for (const auto& m : p->models) 
      {
        for (const auto& t : m->tags) 
        {
          
          size_t iamshort = 5000;
          int chid = 0;
          int counter = 0;

          for (const std::string& alias : tag::tagAliases[t]) {
            
            if (alias.length() < iamshort){
              chid = counter;
              iamshort = alias.length();
            }
            counter++;
          }

          std::string ttag = tag::tagAliases[t][chid];


          //std::string ttag = tag::tagAliases[t][0];
          if (ttag == labelName->text)
          {
            ModuleMenuItem *item1 = new ModuleMenuItem;
            //item1->slot = slot;
            item1->IAM = "MyLittleTags";
            item1->setModule(module);
            item1->text = m->slug;
            item1->rightText = p->slug;
            item1->pluginName = p->slug;
            item1->moduleName = m->slug;
            menu->addChild(item1);
          }
        }
      }
    }
  }

};

struct MyLittleFavoritesWidget : ModuleWidget {

    TextField *textField;
    slotButton *sb[8];
    MyLittleTools *gmodule;

    MyLittleFavoritesWidget(MyLittleTools *module) {

    gmodule = module;
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/MyLittleFavorites.svg")));

    addChild(createWidget<ScrewSilver>(Vec(0, 0)));

    // preset-saving-screw
    screwPlus *sp = createWidget<screwPlus>(Vec(box.size.x - RACK_GRID_WIDTH, 5));
    sp->setModule(module);
    addChild(sp);



    addChild(createWidget<ScrewSilver>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    textField = createWidget<LedDisplayTextField>(Vec(9, 58));
    textField->box.size = Vec(131, 29);
    addChild(textField);

    heartButton *hb = createWidget<heartButton>(Vec(30, 24));
    hb->IAM = "MyLittleFavorites";
    hb->setModule(module);
    addChild(hb);

    presetButton *pb = createWidget<presetButton>(Vec(117, 24));
    pb->setModule(module);
    addChild(pb);

    renameButton *rb = createWidget<renameButton>(Vec(88, 24));
    rb->setModule(module);
    addChild(rb);

    clearButton *cb = createWidget<clearButton>(Vec(59, 24));
    cb->setModule(module);
    addChild(cb);


    int ystart = 105;
    int yjump = 32;

    for (int i = 0; i < 8; i++)
    {
      sb[i] = createWidget<slotButton>(Vec(9, ystart));

      sb[i]->IAM = "MyLittleFavorites";
      sb[i]->setModule(module);
      sb[i]->buttonid = i;
      sb[i]->setLabelName();
      addChild(sb[i]);
      ystart += yjump;
    }
  }


  // needed for resetAction...
  void step() override 
  {
    if (gmodule)
    {
      if (gmodule->_allLabelReset)
      {
        textField->text = "";
        gmodule->_allLabelReset = false;
        for (int i = 0; i < 8; i++)
        {
          sb[i]->labelName->text = "";
        }
      }

      if (gmodule->setTextFieldText)
      {
        gmodule->setTextFieldText = false;
        textField->text = json_string_value(gmodule->textFieldText);
      }

      // fast method for correct preset-saving...
      if (gmodule->_savePreset)
      {
        gmodule->_savePreset = false;
        std::string dir = asset::user("presets");
        system::createDirectory(dir);

        std::string fn = "";

        fn = textField->text;
        fn.erase(std::remove_if( fn.begin(), fn.end(), isForbidden), fn.end());
 
        osdialog_filters *filters = osdialog_filters_parse(PRESET_FILTERS);
        DEFER({
          osdialog_filters_free(filters);
        });

        char *path = osdialog_file(OSDIALOG_SAVE, dir.c_str(), ("mlf_" + fn).c_str(), filters);
        if (!path) {
          // No path selected
          return;
        }
        DEFER({
          free(path);
        });

        std::string pathStr = path;
        std::string extension = string::filenameExtension(string::filename(pathStr));
        if (extension.empty()) {
          pathStr += ".vcvm";
        }

        save(pathStr);
      }

      



    }    

    Widget::step();
  }


  json_t *toJson() override {
    json_t *rootJ = ModuleWidget::toJson();

    // text
    json_object_set_new(rootJ, "text", json_string(textField->text.c_str()));

    return rootJ;
  }

  void fromJson(json_t *rootJ) override {
    ModuleWidget::fromJson(rootJ);

    // text
    json_t *textJ = json_object_get(rootJ, "text");
    if (textJ)
    {
      if (gmodule) {
        gmodule->textFieldText = json_deep_copy(textJ);
        gmodule->setTextFieldText = true;
      }
    }
  }
};


struct MyLittleTagsWidget : ModuleWidget {

    //TextField *textField;
    MyLittleTagsWidget(MyLittleTools *module) {

    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/MyLittleTags.svg")));

    addChild(createWidget<ScrewSilver>(Vec(0, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    /* 
    textField = createWidget<LedDisplayTextField>(Vec(9, 58));
    textField->box.size = Vec(131, 29);
    addChild(textField);
    */

    heartButton *hb = createWidget<heartButton>(Vec(38, 24));
    hb->IAM = "MyLittleTags";
    hb->viewEditMode(false);
    hb->setModule(module);
    addChild(hb);

    int ystart = 80;
    int yjump = 32;

    for (int i = 0; i < 8; i++)
    {
      slotButton *sb;
      sb = createWidget<slotButton>(Vec(9, ystart));
      sb->IAM = "MyLittleTags";

      sb->setModule(module);
      sb->buttonid = i;
      sb->labelName->text = "";
      //sb->setLabelNameTag(module->tags[module->_tagsListJump + i]);
      addChild(sb);
      ystart += yjump;
    }

  }

  /*
  json_t *toJson() override {
    json_t *rootJ = ModuleWidget::toJson();

    // text
    json_object_set_new(rootJ, "text", json_string(textField->text.c_str()));

    return rootJ;
  }

  void fromJson(json_t *rootJ) override {
    ModuleWidget::fromJson(rootJ);

    // text
    json_t *textJ = json_object_get(rootJ, "text");
    if (textJ)
      textField->text = json_string_value(textJ);
  }
  */
};

Model *modelMyLittleFavorites = createModel<MyLittleTools, MyLittleFavoritesWidget>("MyLittleFavorites");
Model *modelMyLittleTags = createModel<MyLittleTools, MyLittleTagsWidget>("MyLittleTags");
