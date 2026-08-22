// EventOrganizer.java
import java.io.*;
import java.nio.file.*;
import java.util.*;
import com.google.gson.*;

class Task {
    String id;
    String description;
    String category;
    boolean completed;

    Task() {}
    Task(String description, String category) {
        this.id = UUID.randomUUID().toString().substring(0,8);
        this.description = description;
        this.category = category;
        this.completed = false;
    }
}

class Event {
    String id;
    String name;
    String date;
    String description;
    List<Task> tasks = new ArrayList<>();

    Event() {}
    Event(String name, String date, String description) {
        this.id = UUID.randomUUID().toString().substring(0,8);
        this.name = name;
        this.date = date;
        this.description = description;
    }
}

class Organizer {
    private List<Event> events = new ArrayList<>();
    private final String dataFile = "events.json";
    private final Gson gson = new GsonBuilder().setPrettyPrinting().create();

    public Organizer() { load(); }

    private void load() {
        try {
            Path path = Paths.get(dataFile);
            if (Files.exists(path)) {
                String json = new String(Files.readAllBytes(path));
                Event[] arr = gson.fromJson(json, Event[].class);
                events = Arrays.asList(arr);
            }
        } catch (Exception e) {}
    }

    private void save() {
        try {
            Files.write(Paths.get(dataFile), gson.toJson(events).getBytes());
        } catch (Exception e) {}
    }

    private Event getEvent(String id) {
        for (Event e : events) if (e.id.equals(id)) return e;
        return null;
    }

    public void create(String name, String date, String desc) {
        Event e = new Event(name, date, desc);
        events.add(e);
        save();
        System.out.printf("✅ Event created: %s (ID: %s)%n", e.name, e.id);
    }

    public void addTask(String eventId, String desc, String category) {
        Event e = getEvent(eventId);
        if (e == null) {
            System.out.printf("Event %s not found.%n", eventId);
            return;
        }
        Task t = new Task(desc, category);
        e.tasks.add(t);
        save();
        System.out.printf("✅ Task added: %s (ID: %s)%n", t.description, t.id);
    }

    public void list() {
        if (events.isEmpty()) {
            System.out.println("No events.");
            return;
        }
        System.out.println("\n📋 Events:");
        for (Event e : events) {
            int total = e.tasks.size();
            int done = 0;
            for (Task t : e.tasks) if (t.completed) done++;
            int pct = total > 0 ? (int)(done * 100.0 / total) : 0;
            System.out.printf("  %s (ID: %s) – %d/%d tasks done (%d%%)%n", e.name, e.id, done, total, pct);
        }
    }

    public void show(String eventId) {
        Event e = getEvent(eventId);
        if (e == null) {
            System.out.printf("Event %s not found.%n", eventId);
            return;
        }
        int total = e.tasks.size();
        int done = 0;
        for (Task t : e.tasks) if (t.completed) done++;
        int pct = total > 0 ? (int)(done * 100.0 / total) : 0;
        System.out.printf("\n📋 Event: %s%n", e.name);
        if (e.date != null && !e.date.isEmpty()) System.out.printf("Date: %s%n", e.date);
        if (e.description != null && !e.description.isEmpty()) System.out.printf("Description: %s%n", e.description);
        System.out.printf("Progress: %d/%d tasks done (%d%%)%n", done, total, pct);
        if (total == 0) {
            System.out.println("No tasks yet.");
            return;
        }
        System.out.println("\nTasks:");
        int i = 1;
        for (Task t : e.tasks) {
            String status = t.completed ? "[X]" : "[ ]";
            String cat = t.category != null && !t.category.isEmpty() ? " (" + t.category + ")" : "";
            System.out.printf("  %d. %s %s%s (ID: %s)%n", i++, status, t.description, cat, t.id);
        }
    }

    public void markDone(String eventId, String taskId) {
        Event e = getEvent(eventId);
        if (e == null) {
            System.out.printf("Event %s not found.%n", eventId);
            return;
        }
        for (Task t : e.tasks) {
            if (t.id.equals(taskId)) {
                if (t.completed) {
                    System.out.println("Task already completed.");
                } else {
                    t.completed = true;
                    save();
                    System.out.printf("✅ Task marked as done: %s%n", t.description);
                }
                return;
            }
        }
        System.out.printf("Task %s not found in this event.%n", taskId);
    }

    public void delete(String eventId) {
        Iterator<Event> it = events.iterator();
        while (it.hasNext()) {
            Event e = it.next();
            if (e.id.equals(eventId)) {
                String name = e.name;
                it.remove();
                save();
                System.out.printf("✅ Event deleted: %s%n", name);
                return;
            }
        }
        System.out.printf("Event %s not found.%n", eventId);
    }

    public static void main(String[] args) {
        if (args.length < 1) {
            System.out.println("Usage: EventOrganizer <command> [options]");
            return;
        }
        Organizer app = new Organizer();
        String cmd = args[0];
        Map<String, String> params = new HashMap<>();
        for (int i=1; i<args.length; i++) {
            if (args[i].startsWith("--") && i+1 < args.length) {
                params.put(args[i].substring(2), args[++i]);
            }
        }
        switch (cmd) {
            case "create":
                if (args.length < 2) { System.out.println("create <name> [--date DATE] [--desc TEXT]"); return; }
                app.create(args[1], params.getOrDefault("date", ""), params.getOrDefault("desc", ""));
                break;
            case "add-task":
                if (args.length < 4) { System.out.println("add-task <event_id> <description> [--category CAT]"); return; }
                app.addTask(args[1], args[2], params.getOrDefault("category", ""));
                break;
            case "list":
                app.list();
                break;
            case "show":
                if (args.length < 2) { System.out.println("show <event_id>"); return; }
                app.show(args[1]);
                break;
            case "done":
                if (args.length < 3) { System.out.println("done <event_id> <task_id>"); return; }
                app.markDone(args[1], args[2]);
                break;
            case "delete":
                if (args.length < 2) { System.out.println("delete <event_id>"); return; }
                app.delete(args[1]);
                break;
            default:
                System.out.println("Unknown command.");
        }
    }
}
