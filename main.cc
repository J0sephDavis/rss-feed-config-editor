#include <cstdlib>
#include <ftxui/component/component_base.hpp>
#include <ftxui/dom/node.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <functional>
#include <logger.cc>
#include <rapidxml.hpp>
#include <rapidxml_utils.hpp>
#include <rapidxml_print.hpp>
#include <vector>
#include <ftxui/dom/table.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
using namespace libLogger;
using namespace ftxui;
namespace rx = rapidxml;
struct config_fields {
	public:
		std::string fileName;
		std::string title;
		std::string regex;
		std::string history;
		std::string url;
		bool save_entry = false; //ONLY USED FOR NON-REFERENCE i.e., NEW entries
		friend std::ostream& operator<<(std::ostream& os,
				const config_fields& entry) {
			os << "TITLE: " << entry.title << "\n"
				<< "FILE: " << entry.fileName << "\n"
				<< "EXPR: " << entry.regex << "\n"
				<< "HIST: " << entry.history << "\n"
				<< "URL: " << entry.url;
			return os;
		}
};
//a feed with an already existing reference
class config_entry {
	public:
		config_entry(rx::xml_node<>& reference) :
			xml_reference(reference)
		{
			log.trace("config_entry constructor");
			std::string fileName = reference.first_node("feedFileName")->value();
			std::string title = reference.first_node("title")->first_node()->value();
			std::string regex = reference.first_node("expr")->value();
			std::string history = reference.first_node("history")->value();
			std::string url = reference.first_node("feed-url")->first_node()->value();
			original_entry = {
				fileName,title,regex,history,url
			};
			changed_entry = original_entry;
		}
		~config_entry() {
			log.debug("config_entry::deconstructor");
		}
	public: //assistant
		std::string str() {
			std::string return_value = "TITLE: " + original_entry.title + "\n"
				+ "FILE: " + original_entry.fileName + "\n"
				+ "EXPR: " + original_entry.regex + "\n"
				+ "HIST: " + original_entry.history + "\n"
				+ "URL: " + original_entry.url;
			return return_value;
		}
	public: //resets
		void r_fileName() {this->changed_entry.fileName = original_entry.fileName; }
		void r_title() { this->changed_entry.title = original_entry.title; }
		void r_regex() { this->changed_entry.regex = original_entry.regex; }
		void r_history() { this->changed_entry.history = original_entry.history; }
		void r_url() { this->changed_entry.url = original_entry.url; }
	public: //GETTERS
		std::string g_fileName() { return original_entry.fileName; }
		std::string g_title() { return original_entry.title; }
		std::string g_regex() { return original_entry.regex; }
		std::string g_history() { return original_entry.history; }
		std::string g_url() { return original_entry.url; }
		const rx::xml_node<>& g_xmlRef() const {
			return xml_reference;
		}
	public: //other
		//tmp_xxx are used to update values
		config_fields changed_entry;
		bool update_fileName = false;
		bool update_title = false;
		bool update_regex = false;
		bool update_history = false;
		bool update_url = false;
	private: //DESCRIPTORS
		const rx::xml_node<>& xml_reference;
		config_fields original_entry;
};
class feed_editor : public ComponentBase {
public:
	explicit feed_editor(config_entry& entry,
			int& menu_column,
			int& menu_row)  {
		log.trace("feed_editor constructor");
		Add(Container::Vertical({
			compose_line_item(
				Input(&(entry.changed_entry.title),
					entry.g_title()),
				Checkbox("Update?", (&entry.update_title)),
				Button("Reset", [&](){ (&entry)->r_title();},
					resetBtnOpt), menu_column
			),
			compose_line_item(
				Input(&(entry.changed_entry.fileName),
					entry.g_fileName()),
				Checkbox("Update?", (&entry.update_fileName)),
				Button("Reset", [&](){ (&entry)->r_fileName();},
					resetBtnOpt), menu_column
			),
			compose_line_item(
				Input(&(entry.changed_entry.regex),
					entry.g_regex()),
				Checkbox("Update?", (&entry.update_regex)),
				Button("Reset", [&](){ (&entry)->r_regex();},
					resetBtnOpt), menu_column
			),
			compose_line_item(
				Input(&(entry.changed_entry.history),
					entry.g_history()),
				Checkbox("Update?", (&entry.update_history)),
				Button("Reset", [&](){ (&entry)->r_history();},
					resetBtnOpt), menu_column
			),
			compose_line_item(
				Input(&(entry.changed_entry.url),
					entry.g_url()),
				Checkbox("Update?", (&entry.update_url)),
				Button("Reset", [&](){ (&entry)->r_url();},
					resetBtnOpt), menu_column
			),
		}, &menu_row));
	}
private:
	const ButtonOption resetBtnOpt = ButtonOption::Ascii();
	const ComponentDecorator line_item_decorator = Renderer(border);
	//TODO make this more general, stop accepting components
	//instead, build them in the function.
	Component compose_line_item(Component input_box,
				Component checkbox, Component button,
				int& menu_column) {
		return Container::Horizontal({
			input_box, checkbox, button
		}, &menu_column) | line_item_decorator;
	};
};
class new_feed_editor : public ComponentBase {
public:
	explicit new_feed_editor(config_fields& entry_ref, int& menu_row):
		entry_contents(entry_ref){
		log.trace("new_feed_editor constructor");
		Add(Container::Vertical({
			Input(&entry_contents.fileName, "file name"),
			Input(&entry_contents.title, "title"),
			Input(&entry_contents.regex, "regex"),
			Input(&entry_contents.history, "history"),
			Input(&entry_contents.url, "url"),
			Checkbox("Save?", &entry_contents.save_entry)
		}, &menu_row));
	}
	config_fields& entry_contents;
private:
	const ComponentDecorator line_item_decorator = Renderer(border);
	Component compose_line_item(Component input_box,
				Component checkbox, Component button,
				int& menu_column) {
		return Container::Horizontal({
			input_box, checkbox, button
		}, &menu_column) | line_item_decorator;
	};
};
//to create components from out component classes
Component editor_comp (config_entry& entry, int& menu_column, int& menu_row) {
	return Make<feed_editor>(entry, menu_column, menu_row);
}
Component new_editor_comp (config_fields& field_data, int& menu_row) {
	return Make<new_feed_editor>(field_data, menu_row);
}
int main(int argc, char* argv[]) {
	if (argc < 1) exit(EXIT_FAILURE);
	std::filesystem::path path_to_config(argv[1]);
	if (!path_to_config.has_extension())
		exit(EXIT_FAILURE);
	if (path_to_config.extension() != ".xml")
		exit(EXIT_FAILURE);
//	const std::string path_to_config = "/home/sooth/Documents/Code/10-19/11/04 RSS-Feed config editor/data/rss-config.xml";
	static rx::xml_document<> config_document;
	static rx::file<> config_file(path_to_config);
	try {
		log.trace("parce config");
		config_document.parse<0>(config_file.data());
	}
	catch (rx::parse_error &e) {
		log.error("Failed to parse config:" + std::string(e.what()));
		exit(EXIT_FAILURE);
	}
	// store the config data
	std::vector<config_entry> cfg_entries;
	cfg_entries.reserve(50); //avoid resizing vector
	for (rx::xml_node<>* entry_node
		= config_document.first_node()->first_node("item");
			entry_node;
			entry_node = entry_node->next_sibling()) {
		log.trace("<loop> store config data");
		cfg_entries.emplace_back(*entry_node);
		log.debug("New entry:" + cfg_entries.back().str());
	}
	// create each tab
	std::vector<std::string> tab_menu_entries;
	std::vector<Component> tab_data;
	int editor_menu_column = 0;
	int editor_menu_row = 0;
	//reset button option
	for (auto& entry : cfg_entries) {
		log.trace("<loop> add tab");
		tab_data.push_back(std::move(
			editor_comp(entry,
				editor_menu_column, editor_menu_row))
		);
		tab_menu_entries.push_back(entry.g_title());
	}
	// create the final tab (adds a new row)
	int counter = 0;
	int tab_selector = 0;
	// bundle tabs
	auto tabs = Container::Tab(tab_data,&tab_selector);
	Component main_component;
	Component tab_menu = Menu(&tab_menu_entries, &tab_selector);
	// button to add config tab
	std::vector<config_fields*> added_configs;
	std::function<void()> newfunc([&]{
		log.trace(">new_func");
		added_configs.push_back(new config_fields);
		tab_data.emplace_back(new_editor_comp(*(added_configs.back()),
					editor_menu_row));	
		tabs->Detach(); //Remove from the main_component interaction hierarchy
		tabs = Container::Tab(tab_data, &tab_selector);
		main_component->Add(tabs); //add to the main_component interaction hierarchy
		tab_menu_entries.push_back("new tab #"
			+ std::to_string(counter++));
		return;
	});
	auto add_config_button = Button("New Config", newfunc);
	// to render the screen interactively
	auto screen = ScreenInteractive::FitComponent();
	// used to establish hierarchy of components
	main_component = Container::Horizontal({
			Container::Vertical({
				tab_menu,
				add_config_button,
			}),
			tabs
	});
	//the final rendered component
	auto main_renderer =  Renderer(main_component, [&](){
		return hbox({
			vbox({
				tab_menu->Render(),
				separator(),
				add_config_button->Render(),
			}),
			separator(),
			tabs->Render(),
		}) | border;
	}) | CatchEvent([&](Event event) {
		//prevent quitting while in input component
		if (event == Event::Character('q') && !tabs->Focused()) {
			//TODO prompt user to save or discard changes
			//TODO print summary of changes
			screen.ExitLoopClosure()();
			return true;
		}
		else return false;
	});
	log.trace("SCREEN LOOP");
	screen.Loop(main_renderer);
	bool config_changed = false;
	for (auto& entry : cfg_entries) {
		log.trace("<loop> check entry for change");
		auto& node = entry.g_xmlRef();
		if (entry.update_fileName || entry.update_history
				|| entry.update_regex || entry.update_url
				|| entry.update_title)
			config_changed = true;
		else continue;
		//
		if (entry.update_fileName) {
			log.info("updating fileName (" + entry.g_fileName()
					+ ") -> ("
					+ entry.changed_entry.fileName + ")");
			node.first_node("feedFileName")->first_node()->value(
				config_document.allocate_string(
					entry.changed_entry.fileName.c_str()));
		}
		if (entry.update_title) {
			log.info("updating title (" + entry.g_title()
					+ ") -> (" + entry.changed_entry.title
					+ ")");
			node.first_node("title")->first_node()->value(
				config_document.allocate_string(
					entry.changed_entry.title.c_str()));
		}
		if (entry.update_regex) {
			log.info("updating regex (" + entry.g_regex()
					+ ") -> (" + entry.changed_entry.regex
					+ ")");
			node.first_node("expr")->first_node()->value(
				config_document.allocate_string(
					entry.changed_entry.regex.c_str()));
		}
		if (entry.update_history) {
			log.info("updating history (" + entry.g_history()
					+ ") -> ("+ entry.changed_entry.history
					+ ")");
			node.first_node("history")->first_node()->value(
				config_document.allocate_string(
					entry.changed_entry.history.c_str()));
		}
		if (entry.update_url) {
			log.info("updating url (" + entry.g_url()
					+ ") -> (" + entry.changed_entry.url
					+ ")");
			node.first_node("feed-url")->first_node()->value(
				config_document.allocate_string(
					entry.changed_entry.url.c_str()));
		}
		//
		log.debug("entry updated:" + entry.str());
	}
	for (auto& entry : added_configs) {
		log.trace("<loop> new config entry:");
		log.debug("\tTITLE:" + entry->title);
		log.debug("\tFILENAME:" + entry->fileName);
		log.debug("\tREGEX:" + entry->regex);
		log.debug("\tHISTORY:" + entry->history);
		log.debug("\tURL:" + entry->url);
		log.debug("\tSAVE?:" + std::string(
			(entry->save_entry)?"true":"false")
		);
		if (entry->save_entry) {
			log.trace("attempt to save entry");
			auto item_node = config_document.allocate_node(
				rx::node_element, "item");
			//allocate values
			auto new_title = config_document.allocate_string(
					entry->title.c_str());
			auto new_fileName = config_document.allocate_string(
					entry->fileName.c_str());
			auto new_url = config_document.allocate_string(
					entry->url.c_str());
			auto new_regex = config_document.allocate_string(
					entry->regex.c_str());
			auto new_history = config_document.allocate_string(
					entry->history.c_str());
			std::cout << "DONE ALLOCATING STRINGS";
			//allocate nodes
			auto title_node = config_document.allocate_node(
				rx::node_element, "title", new_title);
			auto fileName_node = config_document.allocate_node(
				rx::node_element, "feedFileName",
				new_fileName);
			auto url_node = config_document.allocate_node(
				rx::node_element, "feed-url");
			{
				auto url_cdata = config_document.allocate_node(
					rx::node_cdata, 0, new_url);
				url_node->append_node(url_cdata);
			}
			auto regex_node = config_document.allocate_node(
				rx::node_element, "expr", new_regex);
			auto history_node = config_document.allocate_node(
				rx::node_element, "history", new_history);
			std::cout << "allocated each node";
			//append each node to the item node
			item_node->append_node(title_node);
			item_node->append_node(fileName_node);
			item_node->append_node(url_node);
			item_node->append_node(regex_node);
			item_node->append_node(history_node);
			//append the item node to the main XML doc
			config_document.first_node()->append_node(item_node);
		}
	}
	std::ofstream new_config(path_to_config);
	new_config << config_document; //rapidxml_print.hpp

	return EXIT_SUCCESS;
}
