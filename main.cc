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
class feed_entry {
	public:
		feed_entry(rx::xml_node<>& reference) :
			xml_reference(reference)
		{
			log.trace("feed_entry constructor");
			fileName = reference.first_node("feedFileName")->value();
			title = reference.first_node("title")->first_node()->value();
			regex = reference.first_node("expr")->value();
			history = reference.first_node("history")->value();
			url = reference.first_node("feed-url")->first_node()->value();
			//
			this->tmp_fileName = fileName;
			this->tmp_title = title;
			this->tmp_regex = regex;
			this->tmp_history = history;
			this->tmp_url = url;
		}
		~feed_entry() {
			log.debug("feed_entry::deconstructor");
		}
	public: //resets
		void r_fileName() { this->tmp_fileName = fileName; }
		void r_title() { this->tmp_title = title; }
		void r_regex() { this->tmp_regex = regex; }
		void r_history() { this->tmp_history = history; }
		void r_url() { this->tmp_url = url; }
	public: //GETTERS
		std::string g_fileName() const { return fileName; }
		std::string g_title() const { return title; }
		std::string g_regex() const { return regex; }
		std::string g_history() const { return history; }
		std::string g_url() const { return url; }
		const rx::xml_node<>& g_xmlRef() const {
			return xml_reference;
		}
	public: //other
		std::string str() {
			std::string return_value = "TITLE: " + g_title() + "\n"
				+ "FILE: " + g_fileName() + "\n"
				+ "EXPR: " + g_regex() + "\n"
				+ "HIST: " + g_history() + "\n"
				+ "URL: " + g_url();
			return return_value;
		}
		friend std::ostream& operator<<(std::ostream& os,
				const feed_entry& entry) {
			os << "TITLE: " << entry.g_title() << "\n"
				<< "FILE: " << entry.g_fileName() << "\n"
				<< "EXPR: " << entry.g_regex() << "\n"
				<< "HIST: " << entry.g_history() << "\n"
				<< "URL: " << entry.g_url();
			return os;
		}
		//tmp_xxx are used to update values
		std::string tmp_fileName;
		std::string tmp_title;
		std::string tmp_regex;
		std::string tmp_history;
		std::string tmp_url;
		bool changed_fileName = false;
		bool changed_title = false;
		bool changed_regex = false;
		bool changed_history = false;
		bool changed_url = false;
	private: //DESCRIPTORS
		std::string fileName;
		std::string title;
		std::string regex;
		std::string history;
		std::string url;
		//changed = true if any of the descriptors are updated
		const rx::xml_node<>& xml_reference;
};
class feed_editor : public feed_entry, public ComponentBase {
public:
	explicit feed_editor(feed_entry& entry,
			int& menu_column,
			int& menu_row): feed_entry(entry) {
		log.trace("feed_editor constructor");
		Add(Container::Vertical({
			compose_line_item(
				Input(&(entry.tmp_title), entry.g_title()),
				Checkbox("Update?", (&entry.changed_title)),
				Button("Reset", [&](){ (&entry)->r_title();},
					resetBtnOpt), menu_column
			),
			compose_line_item(
				Input(&(entry.tmp_fileName), entry.g_fileName()),
				Checkbox("Update?", (&entry.changed_fileName)),
				Button("Reset", [&](){ (&entry)->r_fileName();},
					resetBtnOpt), menu_column
			),
			compose_line_item(
				Input(&(entry.tmp_regex), entry.g_regex()),
				Checkbox("Update?", (&entry.changed_regex)),
				Button("Reset", [&](){ (&entry)->r_regex();},
					resetBtnOpt), menu_column
			),
			compose_line_item(
				Input(&(entry.tmp_history), entry.g_history()),
				Checkbox("Update?", (&entry.changed_history)),
				Button("Reset", [&](){ (&entry)->r_history();},
					resetBtnOpt), menu_column
			),
			compose_line_item(
				Input(&(entry.tmp_url), entry.g_url()),
				Checkbox("Update?", (&entry.changed_url)),
				Button("Reset", [&](){ (&entry)->r_url();},
					resetBtnOpt), menu_column
			),
		}, &menu_row));
	}
private:
	const ButtonOption resetBtnOpt = ButtonOption::Ascii();
	const ComponentDecorator line_item_decorator = Renderer(border);
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
	explicit new_feed_editor(int& menu_row) {
		log.trace("new_feed_editor constructor");
		Add(Container::Vertical({
			Input(fileName, "file name"),
			Input(title, "title"),
			Input(regex, "regex"),
			Input(history, "history"),
			Input(url, "url"),
			Checkbox("Save?", &mark_saved)
		}, &menu_row));
//			Renderer([](){return text("NEW ENTRY") | hcenter;}),
	}
private:
	bool mark_saved;
	std::string fileName;
	std::string title;
	std::string regex;
	std::string history;
	std::string url;
	const ComponentDecorator line_item_decorator = Renderer(border);
	Component compose_line_item(Component input_box,
				Component checkbox, Component button,
				int& menu_column) {
		return Container::Horizontal({
			input_box, checkbox, button
		}, &menu_column) | line_item_decorator;
	};
};
Component editor_comp (feed_entry& entry, int& menu_column, int& menu_row) {
	return Make<feed_editor>(entry, menu_column, menu_row);
}
Component new_editor_comp (int& menu_row) {
	return Make<new_feed_editor>(menu_row);
}
int main(void) {
	const std::string path_to_config = "/home/sooth/Documents/Code/10-19/11/04 RSS-Feed config editor/data/rss-config.xml";
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
	std::vector<feed_entry> entries;
	entries.reserve(50); //if value is too low,
			     //we waste time resizing the vector
	for (rx::xml_node<>* entry_node = config_document.first_node()->first_node("item");
			entry_node;
			entry_node = entry_node->next_sibling()) {
		log.trace("<loop> store config data");
		entries.emplace_back(*entry_node);
		log.debug("New entry:" + entries.back().str());
	}
	// create each tab
	std::vector<std::string> tab_menu_entries;
	std::vector<Component> tab_data;
	int editor_menu_column = 0;
	int editor_menu_row = 0;
	//reset button option
	const auto resetBtnOpt = ButtonOption::Ascii();
	const ComponentDecorator line_item_decorator = Renderer(border);
	for (auto& entry : entries) {
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
	std::function<void()> newfunc([&]{
		log.trace(">new_func");
		tab_data.emplace_back(new_editor_comp(editor_menu_row));	
		tabs->Detach();
		tabs = Container::Tab(tab_data, &tab_selector);
		main_component->Add(tabs);
		tab_menu_entries.push_back("new tab #" + std::to_string(counter++));
		return;
	});
	auto add_config_button = Button("Create config?", newfunc);
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
	auto x =  Renderer(main_component, [&](){
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
	screen.Loop(x);
	bool config_changed = false;
	for (auto& entry : entries) {
		log.trace("<loop> check entry for change");
		auto& node = entry.g_xmlRef();
		if (entry.changed_fileName || entry.changed_history || entry.changed_regex || entry.changed_url || entry.changed_title)
			config_changed = true;
		else continue;
		//
		if (entry.changed_fileName) {
			log.info("updating fileName (" + entry.g_fileName()
					+ ") -> (" + entry.tmp_fileName + ")");
			node.first_node("feedFileName")->first_node()->value(config_document.allocate_string(entry.tmp_fileName.c_str()));
		}
		if (entry.changed_title) {
			log.info("updating title (" + entry.g_title()
					+ ") -> (" + entry.tmp_title + ")");
			node.first_node("title")->first_node()->value(config_document.allocate_string(entry.tmp_title.c_str()));
		}
		if (entry.changed_regex) {
			log.info("updating regex (" + entry.g_regex()
					+ ") -> (" + entry.tmp_regex + ")");
			node.first_node("expr")->first_node()->value(config_document.allocate_string(entry.tmp_regex.c_str()));
		}
		if (entry.changed_history) {
			log.info("updating history (" + entry.g_history()
					+ ") -> (" + entry.tmp_history + ")");
			node.first_node("history")->first_node()->value(config_document.allocate_string(entry.tmp_history.c_str()));
		}
		if (entry.changed_url) {
			log.info("updating url (" + entry.g_url()
					+ ") -> (" + entry.tmp_url + ")");
			node.first_node("feed-url")->first_node()->value(config_document.allocate_string(entry.tmp_url.c_str()));

		}
		//
		log.debug("entry updated:" + entry.str());
	}
	std::ofstream new_config(path_to_config);
	new_config << config_document; //rapidxml_print.hpp

	return EXIT_SUCCESS;
}
