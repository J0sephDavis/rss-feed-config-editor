#include <cstdlib>
#include <ftxui/dom/node.hpp>
#include <ftxui/screen/screen.hpp>
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
	public: //other
		//returns a vector of elements with the entry descriptors
		std::vector<Element> to_row() {
			std::vector<Element> row;
			row.push_back(text(title));
			row.push_back(text(fileName));
			row.push_back(text(regex));
		//	row.push_back(text(history));
		//	row.push_back(text(url));
			return row;
		};
		friend std::ostream& operator<<(std::ostream& os,
				const feed_entry& entry) {
			os << "TITLE: " << entry.g_title() << "\n"
				<< "FILE: " << entry.g_fileName() << "\n"
				<< "EXPR: " << entry.g_regex() << "\n"
				<< "HIST: " << entry.g_history() << "\n"
				<< "URL: " << entry.g_url();
			return os;
		}
	private: //DESCRIPTORS
		std::string fileName;
		std::string title;
		std::string regex;
		std::string history;
		std::string url;
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
		feed_entry tmp_entry;
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
	// prepare table
	std::vector<std::vector<Element>> table_data;
	table_data.push_back({text("Title"),
			text("File Name"),
			text("Regular Expression")});
	for (auto e : entries) {
//		std::cout << e << "\n---\n";
		table_data.push_back(e.to_row());
	}
	auto table = Table(std::move(table_data));
	// decorate table
	table.SelectAll().Border(HEAVY);
	table.SelectAll().SeparatorVertical(LIGHT);
	table.SelectRow(0).Border(DOUBLE);
	// render screen
	auto document = table.Render();
	auto screen = Screen::Create(Dimension::Fit(document));
	Render(screen, document);
	screen.Print();
	std::cout << std::endl;
	std::ofstream new_config("modified_xml");
	new_config << config_document; //rapidxml_print.hpp

	return 0;
}
