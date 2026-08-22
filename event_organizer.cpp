// event_organizer.cpp
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

class Task {
public:
    string id;
    string description;
    string category;
    bool completed;

    Task() : completed(false) {}
    Task(const string& desc, const string& cat = "") : description(desc), category(cat), completed(false) {
        id = generateId();
    }

    string generateId() {
        const char* hex = "0123456789abcdef";
        string id;
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(0, 15);
        for (int i=0; i<8; i++) id += hex[dis(gen)];
        return id;
    }

    json toJson() const {
        return {{"id", id}, {"description", description}, {"category", category}, {"completed", completed}};
    }

    static Task fromJson(const json& j) {
        Task t;
        t.id = j["id"];
        t.description = j["description"];
        t.category = j.value("category", "");
        t.completed = j.value("completed", false);
        return t;
    }
};

class Event {
public:
    string id;
    string name;
    string date;
    string description;
    vector<Task> tasks;

    Event() {}
    Event(const string& n, const string& d = "", const string& desc = "") : name(n), date(d), description(desc) {
        id = generateId();
    }

    string generateId() {
        const char* hex = "0123456789abcdef";
        string id;
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(0, 15);
        for (int i=0; i<8; i++) id += hex[dis(gen)];
        return id;
    }

    json toJson() const {
        json tasksJson = json::array();
        for (const auto& t : tasks) tasksJson.push_back(t.toJson());
        return {{"id", id}, {"name", name}, {"date", date}, {"description", description}, {"tasks", tasksJson}};
    }

    static Event fromJson(const json& j) {
        Event e;
        e.id = j["id"];
        e.name = j["name"];
        e.date = j.value("date", "");
        e.description = j.value("description", "");
        for (const auto& t : j["tasks"]) {
            e.tasks.push_back(Task::fromJson(t));
        }
        return e;
    }
};

class Organizer {
private:
    vector<Event> events;
    string dataFile = "events.json";

    void load() {
        ifstream f(dataFile);
        if (!f.is_open()) return;
        json j;
        f >> j;
        for (auto& item : j) {
            events.push_back(Event::fromJson(item));
        }
    }

    void save() {
        json j = json::array();
        for (auto& e : events) j.push_back(e.toJson());
        ofstream f(dataFile);
        f << setw(2) << j << endl;
    }

    Event* getEvent(const string& id) {
        for (auto& e : events) {
            if (e.id == id) return &e;
        }
        return nullptr;
    }

public:
    Organizer() { load(); }

    void create(const string& name, const string& date, const string& desc) {
        Event e(name, date, desc);
        events.push_back(e);
        save();
        cout << "✅ Event created: " << e.name << " (ID: " << e.id << ")\n";
    }

    void addTask(const string& eventId, const string& desc, const string& category) {
        Event* e = getEvent(eventId);
        if (!e) {
            cout << "Event " << eventId << " not found.\n";
            return;
        }
        Task t(desc, category);
        e->tasks.push_back(t);
        save();
        cout << "✅ Task added: " << t.description << " (ID: " << t.id << ")\n";
    }

    void list() {
        if (events.empty()) {
            cout << "No events.\n";
            return;
        }
        cout << "\n📋 Events:\n";
        for (auto& e : events) {
            int total = e.tasks.size();
            int done = 0;
            for (auto& t : e.tasks) if (t.completed) done++;
            int pct = total > 0 ? (int)(done * 100.0 / total) : 0;
            cout << "  " << e.name << " (ID: " << e.id << ") – " << done << "/" << total << " tasks done (" << pct << "%)\n";
        }
    }

    void show(const string& eventId) {
        Event* e = getEvent(eventId);
        if (!e) {
            cout << "Event " << eventId << " not found.\n";
            return;
        }
        int total = e->tasks.size();
        int done = 0;
        for (auto& t : e->tasks) if (t.completed) done++;
        int pct = total > 0 ? (int)(done * 100.0 / total) : 0;
        cout << "\n📋 Event: " << e->name << "\n";
        if (!e->date.empty()) cout << "Date: " << e->date << "\n";
        if (!e->description.empty()) cout << "Description: " << e->description << "\n";
        cout << "Progress: " << done << "/" << total << " tasks done (" << pct << "%)\n";
        if (total == 0) {
            cout << "No tasks yet.\n";
            return;
        }
        cout << "\nTasks:\n";
        int i = 1;
        for (auto& t : e->tasks) {
            string status = t.completed ? "[X]" : "[ ]";
            string cat = t.category.empty() ? "" : " (" + t.category + ")";
            cout << "  " << i++ << ". " << status << " " << t.description << cat << " (ID: " << t.id << ")\n";
        }
    }

    void markDone(const string& eventId, const string& taskId) {
        Event* e = getEvent(eventId);
        if (!e) {
            cout << "Event " << eventId << " not found.\n";
            return;
        }
        for (auto& t : e->tasks) {
            if (t.id == taskId) {
                if (t.completed) {
                    cout << "Task already completed.\n";
                } else {
                    t.completed = true;
                    save();
                    cout << "✅ Task marked as done: " << t.description << "\n";
                }
                return;
            }
        }
        cout << "Task " << taskId << " not found in this event.\n";
    }

    void del(const string& eventId) {
        for (auto it = events.begin(); it != events.end(); ++it) {
            if (it->id == eventId) {
                string name = it->name;
                events.erase(it);
                save();
                cout << "✅ Event deleted: " << name << "\n";
                return;
            }
        }
        cout << "Event " << eventId << " not found.\n";
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: event_organizer <command> [options]\n";
        return 1;
    }
    Organizer app;
    string cmd = argv[1];

    if (cmd == "create") {
        if (argc < 3) { cerr << "create <name> [--date DATE] [--desc TEXT]\n"; return 1; }
        string name = argv[2];
        string date, desc;
        for (int i=3; i<argc; i++) {
            if (string(argv[i]) == "--date" && i+1 < argc) date = argv[++i];
            if (string(argv[i]) == "--desc" && i+1 < argc) desc = argv[++i];
        }
        app.create(name, date, desc);
    } else if (cmd == "add-task") {
        if (argc < 4) { cerr << "add-task <event_id> <description> [--category CAT]\n"; return 1; }
        string eventId = argv[2];
        string desc = argv[3];
        string category;
        for (int i=4; i<argc; i++) {
            if (string(argv[i]) == "--category" && i+1 < argc) category = argv[++i];
        }
        app.addTask(eventId, desc, category);
    } else if (cmd == "list") {
        app.list();
    } else if (cmd == "show") {
        if (argc < 3) { cerr << "show <event_id>\n"; return 1; }
        app.show(argv[2]);
    } else if (cmd == "done") {
        if (argc < 4) { cerr << "done <event_id> <task_id>\n"; return 1; }
        app.markDone(argv[2], argv[3]);
    } else if (cmd == "delete") {
        if (argc < 3) { cerr << "delete <event_id>\n"; return 1; }
        app.del(argv[2]);
    } else {
        cerr << "Unknown command.\n";
        return 1;
    }
    return 0;
}
