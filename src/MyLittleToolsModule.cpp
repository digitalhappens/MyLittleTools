#include "MyLittleTools.hpp"
#include "plugin.hpp"
#include "window.hpp"
#include <algorithm>
#include <functional>
#include <random>
#include <libgen.h> // for dirname and basename
#include <ui/Tooltip.hpp>


/*
  Features to be added:

  * renameing favorties (so this makes it a module/preset browser)
  * make it 8x8 bank in one module


*/


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

  json_t *_jsondata[8];
  json_t *_json[8];

  json_t *_jsondata0;
  json_t *_json0;

  int _tagsListJump;

  int _slot;
  bool _editMode;
  bool _bTagsListed;
  std::string tags[100];
  int tagsCount;

  MyLittleTools() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    _plugin = new std::string[8];
    _module = new std::string[8];

    _editMode = false;
    _tagsListJump = 0;
    _bTagsListed = false;
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
      json_object_set_new(jt, "params", _json[_slot] );
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
    for (const std::string &tag : plugin::allowedTags) {
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
            if (!t.compare(trim(tag))) 
            {
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

  std::string getSavedPlugin(int index)
  {
    return _plugin[index];
  }

  void setSavedModule(int index, std::string module)
  {
    _module[index] = module;

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
    for (int i = 0; i < 8; i++) {
      setSavedPlugin(i, "");
      setSavedModule(i, "");
      _json[i] = NULL;
      _jsondata[i] = NULL;
    }
  }

  json_t *dataToJson() override {
      json_t *rootJ = json_object();

      for (int i = 0; i < 8; i++)
      {
        if (_plugin[i] != "" && _module[i] != "") 
        {
          json_object_set_new(rootJ, convertAndCombine("plugin", i), json_string(_plugin[i].c_str()));
          json_object_set_new(rootJ, convertAndCombine("module", i), json_string(_module[i].c_str()));

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

      if (plug)
      {
        _plugin[i] = json_string_value(plug);
      }

      if (mod)
        _module[i] = json_string_value(mod);


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

  presetButton() {
    svg1 = APP->window->loadSvg(asset::plugin(pluginInstance, "res/preset1.svg"));
    addFrame(svg1);
  }

  void setModule(MyLittleTools *module) {
    this->module = module;
  }

  virtual void onAction(const event::Action &e) override {
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


struct heartButton : SvgButton {
    MyLittleTools *module;

    std::shared_ptr<Svg> svg1;
    std::shared_ptr<Svg> svg2;

    ui::Label *labelEditMode;
    std::string IAM;

    std::vector<std::string> presetPaths;

    heartButton() {
      svg1 = APP->window->loadSvg(asset::plugin(pluginInstance, "res/heart1.svg"));
      svg2 = APP->window->loadSvg(asset::plugin(pluginInstance, "res/heart2.svg"));

      addFrame(svg1);
      addFrame(svg2);

    labelEditMode = new ui::Label;
    // brandLabel->fontSize = 16;
    labelEditMode->box.pos.x = 22;
    labelEditMode->box.pos.y = 0;
    labelEditMode->color = nvgRGB(0x30, 0x30, 0x30);
    //labelEditMode->text = "edit mode";
    addChild(labelEditMode);
    viewEditMode(false);

    }

  void setModule(MyLittleTools *module) {
    this->module = module;
  }

  void viewEditMode(bool mode)
  {
    if (IAM == "MyLittleFavorites")
    {
      if (mode)
        labelEditMode->text = "edit mode";
      else
        labelEditMode->text = "<- edit";
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
        }
        else
        {
          module->setEditMode(true);
          frames[0] = svg2;
          viewEditMode(true);
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

struct slotButton : SvgButton {
  MyLittleTools *module;
  int buttonid;
  ui::Label *labelName;
  std::string IAM;

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
       /* if (!module->_bTagsListed)
        {
          module->listTags();
          module->_bTagsListed = true;
        }
        */
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
      unsigned sz = str.size();
      if (sz > 15)
        str.resize(sz+3,'.');

      if (labelName->text != str)
        labelName->text = str;
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

    if (IAM == "MyLittleTags")
    {
      if (labelName->text != "")
        makeContextMenuOnButton_tag(buttonid);
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
          if (t == labelName->text) 
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

    MyLittleFavoritesWidget(MyLittleTools *module) {

    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/MyLittleFavorites.svg")));

    addChild(createWidget<ScrewSilver>(Vec(0, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    textField = createWidget<LedDisplayTextField>(Vec(9, 58));
    textField->box.size = Vec(131, 29);
    addChild(textField);

    heartButton *hb = createWidget<heartButton>(Vec(38, 24));
    hb->IAM = "MyLittleFavorites";
    hb->setModule(module);
    addChild(hb);

    presetButton *pb = createWidget<presetButton>(Vec(117, 24));
    pb->setModule(module);
    addChild(pb);

    //hb->presetPaths = this->model->presetPaths;
      /*
    if (!model->presetPaths.empty())
      INFO("nicht empty");
    else
      INFO("empty");*/
   // for (const std::string &presetPath : this->model->presetPaths) {
   //   INFO(string::filename(presetPath).c_str());
   // } 


    

    int ystart = 105;
    int yjump = 32;

    for (int i = 0; i < 8; i++)
    {
      slotButton *sb;
      sb = createWidget<slotButton>(Vec(9, ystart));

      sb->IAM = "MyLittleFavorites";
      sb->setModule(module);
      sb->buttonid = i;
      sb->setLabelName();
      addChild(sb);
      ystart += yjump;
    }
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
      textField->text = json_string_value(textJ);
  }
};


struct MyLittleTagsWidget : ModuleWidget {

    TextField *textField;
    

    MyLittleTagsWidget(MyLittleTools *module) {

    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/MyLittleTags.svg")));

    addChild(createWidget<ScrewSilver>(Vec(0, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    textField = createWidget<LedDisplayTextField>(Vec(9, 58));
    textField->box.size = Vec(131, 29);
    addChild(textField);

    heartButton *hb = createWidget<heartButton>(Vec(38, 24));
    hb->IAM = "MyLittleTags";
    hb->viewEditMode(false);
    hb->setModule(module);
    addChild(hb);

    int ystart = 105;
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
};

Model *modelMyLittleFavorites = createModel<MyLittleTools, MyLittleFavoritesWidget>("MyLittleFavorites");
Model *modelMyLittleTags = createModel<MyLittleTools, MyLittleTagsWidget>("MyLittleTags");
