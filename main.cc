#include <cstdlib>
#include <ftxui/dom/node.hpp>
#include <ftxui/component/screen_interactive.hpp>
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
		feed_entry(rx::xml_node<>& reference):
			xml_reference(reference)
		{
			//
		}
		~feed_entry() {
			log.debug("feed_entry::deconstructor");
			if (changed_fileName)
				log.debug("FILE:" + fileName);
			if (changed_title)
				log.debug("TITLE:" + title);
			if (changed_regex)
				log.debug("REGEX:" + regex);
			if (changed_history)
				log.debug("HISTORY:" + history);
			if (changed_url)
				log.debug("URL:" + url);
			log.debug("return feed_entry::deconstructor");
		}
	public: //updaters
		void u_fileName() {
			this->fileName = tmp_fileName;
			changed_fileName = true;
		}
		void u_title() {
			this->title = tmp_title;
			changed_title = true;
		}
		void u_regex() {
			this->regex = tmp_regex;
			changed_regex = true;
		}
		void u_history() {
			this->history = tmp_history;
			changed_history = true;
		}
		void u_url() {
			this->url = tmp_url;
			changed_url = true;
		}
	public: //SETTERS
		void s_fileName(std::string file_name) {
			this->fileName = file_name;
		}
		void s_title(std::string feed_title) {
			this->title = feed_title;
		}
		void s_regex(std::string feed_expression) {
			this->regex = feed_expression;
		}
		void s_history(std::string feed_history) {
			this->history = feed_history;
		}
		void s_url(std::string feed_url) {
			this->url = feed_url;
		}
	public: //GETTERS
		std::string g_fileName() const {
			return fileName;
		}
		std::string g_title() const {
			return title;
		}
		std::string g_regex() const {
			return regex;
		}
		std::string g_history() const {
			return history;
		}
		std::string g_url() const {
			return url;
		}
		const rx::xml_node<>& g_xmlRef() const {
			return xml_reference;
		}
	public: //other
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
		bool changed = false; //use deconstructor to allocate the XML
		const rx::xml_node<>& xml_reference;
};
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
	for (auto entry_node = config_document.first_node()->first_node("item");
			entry_node;
			entry_node = entry_node->next_sibling()) {
		feed_entry tmp_entry(*entry_node);
		tmp_entry.s_fileName(
			entry_node->first_node("feedFileName")->value()
		);
		tmp_entry.s_history(
			entry_node->first_node("history")->value()
		);
		tmp_entry.s_title(
			entry_node->first_node("title")->first_node()->value()
		);
		tmp_entry.s_url(
			entry_node->first_node("feed-url")->first_node()->value()
		);
		tmp_entry.s_regex(
			entry_node->first_node("expr")->value()
		);
		entries.emplace_back(std::move(tmp_entry));
	}
	// create each tab
	std::vector<std::string> tab_menu_entries;
	std::vector<Component> tab_data;
	for (auto& entry : entries) {
		Component return_value = Container::Vertical({
			Container::Horizontal({
				Input(&(entry.tmp_title), entry.g_title()),
				Button("Update", [&](){ (&entry)->u_title();}),
			}),
			Container::Horizontal({
				Input(&(entry.tmp_fileName), entry.g_fileName()),
				Button("Update", [&](){ (&entry)->u_fileName();}),
			}),
			Container::Horizontal({
				Input(&(entry.tmp_regex), entry.g_regex()),
				Button("Update", [&](){ (&entry)->u_regex();}),
			}),
			Container::Horizontal({
				Input(&(entry.tmp_history), entry.g_history()),
				Button("Update", [&](){ (&entry)->u_history();}),
			}),
			Container::Horizontal({
				Input(&(entry.tmp_url), entry.g_url()),
				Button("Update", [&](){ (&entry)->u_url();}),
			}),
		});
		tab_data.push_back(std::move(return_value));
		tab_menu_entries.push_back(entry.g_title());
	}
	// bundle tabs
	int tab_selector = 0;
	auto tabs = Container::Tab(tab_data,&tab_selector);
	Component tab_menu = Menu(&tab_menu_entries, &tab_selector);
	// to render the screen interactively
	auto screen = ScreenInteractive::FitComponent();
	// used to establish hierarchy of components
	Component main_component = Container::Horizontal({
		tab_menu, tabs
	});
	//the final rendered component
	auto x =  Renderer(main_component, [&](){
		return vbox({
			text("header"),
			text(std::to_string(tab_selector)),
			separator(),
			hbox({
				tab_menu->Render(),
				separator(),
				tabs->Render() | border,
			}),
			separator(),
			text("footer"),
		});
	});
	log.info("SCREEN LOOP");
	screen.Loop(x);
//	std::ofstream new_config("modified_xml");
//	new_config << config_document; //rapidxml_print.hpp

	return 0;
}
