// EventOrganizer.cs
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.Json;
using System.Text.Json.Serialization;

class Task
{
    [JsonPropertyName("id")]
    public string Id { get; set; }
    [JsonPropertyName("description")]
    public string Description { get; set; }
    [JsonPropertyName("category")]
    public string Category { get; set; }
    [JsonPropertyName("completed")]
    public bool Completed { get; set; }

    public Task() { }
    public Task(string description, string category)
    {
        Id = Guid.NewGuid().ToString().Substring(0,8);
        Description = description;
        Category = category;
        Completed = false;
    }
}

class Event
{
    [JsonPropertyName("id")]
    public string Id { get; set; }
    [JsonPropertyName("name")]
    public string Name { get; set; }
    [JsonPropertyName("date")]
    public string Date { get; set; }
    [JsonPropertyName("description")]
    public string Description { get; set; }
    [JsonPropertyName("tasks")]
    public List<Task> Tasks { get; set; } = new List<Task>();

    public Event() { }
    public Event(string name, string date, string description)
    {
        Id = Guid.NewGuid().ToString().Substring(0,8);
        Name = name;
        Date = date;
        Description = description;
    }
}

class Organizer
{
    private List<Event> events = new List<Event>();
    private readonly string dataFile = "events.json";
    private readonly JsonSerializerOptions options = new JsonSerializerOptions { WriteIndented = true };

    public Organizer() => Load();

    private void Load()
    {
        if (!File.Exists(dataFile)) return;
        string json = File.ReadAllText(dataFile);
        events = JsonSerializer.Deserialize<List<Event>>(json) ?? new List<Event>();
    }

    private void Save()
    {
        string json = JsonSerializer.Serialize(events, options);
        File.WriteAllText(dataFile, json);
    }

    private Event GetEvent(string id) => events.FirstOrDefault(e => e.Id == id);

    public void Create(string name, string date, string desc)
    {
        var e = new Event(name, date, desc);
        events.Add(e);
        Save();
        Console.WriteLine($"✅ Event created: {e.Name} (ID: {e.Id})");
    }

    public void AddTask(string eventId, string desc, string category)
    {
        var e = GetEvent(eventId);
        if (e == null)
        {
            Console.WriteLine($"Event {eventId} not found.");
            return;
        }
        var t = new Task(desc, category);
        e.Tasks.Add(t);
        Save();
        Console.WriteLine($"✅ Task added: {t.Description} (ID: {t.Id})");
    }

    public void List()
    {
        if (!events.Any())
        {
            Console.WriteLine("No events.");
            return;
        }
        Console.WriteLine("\n📋 Events:");
        foreach (var e in events)
        {
            int total = e.Tasks.Count;
            int done = e.Tasks.Count(t => t.Completed);
            int pct = total > 0 ? (int)(done * 100.0 / total) : 0;
            Console.WriteLine($"  {e.Name} (ID: {e.Id}) – {done}/{total} tasks done ({pct}%)");
        }
    }

    public void Show(string eventId)
    {
        var e = GetEvent(eventId);
        if (e == null)
        {
            Console.WriteLine($"Event {eventId} not found.");
            return;
        }
        int total = e.Tasks.Count;
        int done = e.Tasks.Count(t => t.Completed);
        int pct = total > 0 ? (int)(done * 100.0 / total) : 0;
        Console.WriteLine($"\n📋 Event: {e.Name}");
        if (!string.IsNullOrEmpty(e.Date)) Console.WriteLine($"Date: {e.Date}");
        if (!string.IsNullOrEmpty(e.Description)) Console.WriteLine($"Description: {e.Description}");
        Console.WriteLine($"Progress: {done}/{total} tasks done ({pct}%)");
        if (total == 0)
        {
            Console.WriteLine("No tasks yet.");
            return;
        }
        Console.WriteLine("\nTasks:");
        int i = 1;
        foreach (var t in e.Tasks)
        {
            string status = t.Completed ? "[X]" : "[ ]";
            string cat = !string.IsNullOrEmpty(t.Category) ? $" ({t.Category})" : "";
            Console.WriteLine($"  {i++}. {status} {t.Description}{cat} (ID: {t.Id})");
        }
    }

    public void MarkDone(string eventId, string taskId)
    {
        var e = GetEvent(eventId);
        if (e == null)
        {
            Console.WriteLine($"Event {eventId} not found.");
            return;
        }
        var t = e.Tasks.FirstOrDefault(task => task.Id == taskId);
        if (t == null)
        {
            Console.WriteLine($"Task {taskId} not found in this event.");
            return;
        }
        if (t.Completed)
        {
            Console.WriteLine("Task already completed.");
        }
        else
        {
            t.Completed = true;
            Save();
            Console.WriteLine($"✅ Task marked as done: {t.Description}");
        }
    }

    public void Delete(string eventId)
    {
        var e = GetEvent(eventId);
        if (e == null)
        {
            Console.WriteLine($"Event {eventId} not found.");
            return;
        }
        string name = e.Name;
        events.Remove(e);
        Save();
        Console.WriteLine($"✅ Event deleted: {name}");
    }

    static void Main(string[] args)
    {
        if (args.Length < 1)
        {
            Console.WriteLine("Usage: EventOrganizer <command> [options]");
            return;
        }
        var app = new Organizer();
        var cmd = args[0];
        var parsed = ParseArgs(args);
        switch (cmd)
        {
            case "create":
                if (args.Length < 2) { Console.WriteLine("create <name> [--date DATE] [--desc TEXT]"); return; }
                app.Create(args[1], parsed.GetValueOrDefault("date", ""), parsed.GetValueOrDefault("desc", ""));
                break;
            case "add-task":
                if (args.Length < 4) { Console.WriteLine("add-task <event_id> <description> [--category CAT]"); return; }
                app.AddTask(args[1], args[2], parsed.GetValueOrDefault("category", ""));
                break;
            case "list":
                app.List();
                break;
            case "show":
                if (args.Length < 2) { Console.WriteLine("show <event_id>"); return; }
                app.Show(args[1]);
                break;
            case "done":
                if (args.Length < 3) { Console.WriteLine("done <event_id> <task_id>"); return; }
                app.MarkDone(args[1], args[2]);
                break;
            case "delete":
                if (args.Length < 2) { Console.WriteLine("delete <event_id>"); return; }
                app.Delete(args[1]);
                break;
            default:
                Console.WriteLine("Unknown command.");
                break;
        }
    }

    static Dictionary<string, string> ParseArgs(string[] args)
    {
        var dict = new Dictionary<string, string>();
        for (int i = 1; i < args.Length; i++)
        {
            if (args[i].StartsWith("--") && i + 1 < args.Length)
            {
                dict[args[i].Substring(2)] = args[++i];
            }
        }
        return dict;
    }
}
