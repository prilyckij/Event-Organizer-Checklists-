// event_organizer.go
package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"os"
	"time"
	"github.com/google/uuid"
)

type Task struct {
	ID          string `json:"id"`
	Description string `json:"description"`
	Category    string `json:"category"`
	Completed   bool   `json:"completed"`
}

type Event struct {
	ID          string `json:"id"`
	Name        string `json:"name"`
	Date        string `json:"date"`
	Description string `json:"description"`
	Tasks       []Task `json:"tasks"`
}

type Organizer struct {
	Events []Event `json:"events"`
}

var dataFile = "events.json"

func (o *Organizer) load() {
	data, err := os.ReadFile(dataFile)
	if err != nil {
		return
	}
	json.Unmarshal(data, o)
}

func (o *Organizer) save() {
	data, _ := json.MarshalIndent(o, "", "  ")
	os.WriteFile(dataFile, data, 0644)
}

func (o *Organizer) getEvent(id string) *Event {
	for i := range o.Events {
		if o.Events[i].ID == id {
			return &o.Events[i]
		}
	}
	return nil
}

func (o *Organizer) create(name, date, desc string) {
	ev := Event{
		ID:          uuid.New().String()[:8],
		Name:        name,
		Date:        date,
		Description: desc,
	}
	o.Events = append(o.Events, ev)
	o.save()
	fmt.Printf("✅ Event created: %s (ID: %s)\n", ev.Name, ev.ID)
}

func (o *Organizer) addTask(eventID, desc, category string) {
	ev := o.getEvent(eventID)
	if ev == nil {
		fmt.Printf("Event %s not found.\n", eventID)
		return
	}
	task := Task{
		ID:          uuid.New().String()[:8],
		Description: desc,
		Category:    category,
	}
	ev.Tasks = append(ev.Tasks, task)
	o.save()
	fmt.Printf("✅ Task added: %s (ID: %s)\n", task.Description, task.ID)
}

func (o *Organizer) list() {
	if len(o.Events) == 0 {
		fmt.Println("No events.")
		return
	}
	fmt.Println("\n📋 Events:")
	for _, e := range o.Events {
		total := len(e.Tasks)
		done := 0
		for _, t := range e.Tasks {
			if t.Completed {
				done++
			}
		}
		pct := 0.0
		if total > 0 {
			pct = float64(done) / float64(total) * 100
		}
		fmt.Printf("  %s (ID: %s) – %d/%d tasks done (%.0f%%)\n", e.Name, e.ID, done, total, pct)
	}
}

func (o *Organizer) show(eventID string) {
	ev := o.getEvent(eventID)
	if ev == nil {
		fmt.Printf("Event %s not found.\n", eventID)
		return
	}
	total := len(ev.Tasks)
	done := 0
	for _, t := range ev.Tasks {
		if t.Completed {
			done++
		}
	}
	pct := 0.0
	if total > 0 {
		pct = float64(done) / float64(total) * 100
	}
	fmt.Printf("\n📋 Event: %s\n", ev.Name)
	if ev.Date != "" {
		fmt.Printf("Date: %s\n", ev.Date)
	}
	if ev.Description != "" {
		fmt.Printf("Description: %s\n", ev.Description)
	}
	fmt.Printf("Progress: %d/%d tasks done (%.0f%%)\n", done, total, pct)
	if total == 0 {
		fmt.Println("No tasks yet.")
		return
	}
	fmt.Println("\nTasks:")
	for i, task := range ev.Tasks {
		status := "[ ]"
		if task.Completed {
			status = "[X]"
		}
		cat := ""
		if task.Category != "" {
			cat = " (" + task.Category + ")"
		}
		fmt.Printf("  %d. %s %s%s (ID: %s)\n", i+1, status, task.Description, cat, task.ID)
	}
}

func (o *Organizer) markDone(eventID, taskID string) {
	ev := o.getEvent(eventID)
	if ev == nil {
		fmt.Printf("Event %s not found.\n", eventID)
		return
	}
	for i := range ev.Tasks {
		if ev.Tasks[i].ID == taskID {
			if ev.Tasks[i].Completed {
				fmt.Println("Task already completed.")
			} else {
				ev.Tasks[i].Completed = true
				o.save()
				fmt.Printf("✅ Task marked as done: %s\n", ev.Tasks[i].Description)
			}
			return
		}
	}
	fmt.Printf("Task %s not found in this event.\n", taskID)
}

func (o *Organizer) delete(eventID string) {
	idx := -1
	for i, e := range o.Events {
		if e.ID == eventID {
			idx = i
			break
		}
	}
	if idx == -1 {
		fmt.Printf("Event %s not found.\n", eventID)
		return
	}
	name := o.Events[idx].Name
	o.Events = append(o.Events[:idx], o.Events[idx+1:]...)
	o.save()
	fmt.Printf("✅ Event deleted: %s\n", name)
}

func main() {
	if len(os.Args) < 2 {
		fmt.Println("Usage: event_organizer <command> [options]")
		return
	}
	app := &Organizer{}
	app.load()
	cmd := os.Args[1]

	switch cmd {
	case "create":
		createCmd := flag.NewFlagSet("create", flag.ExitOnError)
		name := createCmd.String("name", "", "")
		date := createCmd.String("date", "", "")
		desc := createCmd.String("desc", "", "")
		createCmd.Parse(os.Args[2:])
		if *name == "" {
			fmt.Println("create requires --name")
			return
		}
		app.create(*name, *date, *desc)

	case "add-task":
		addCmd := flag.NewFlagSet("add-task", flag.ExitOnError)
		eventID := addCmd.String("event-id", "", "")
		desc := addCmd.String("desc", "", "")
		category := addCmd.String("category", "", "")
		addCmd.Parse(os.Args[2:])
		if *eventID == "" || *desc == "" {
			fmt.Println("add-task requires --event-id and --desc")
			return
		}
		app.addTask(*eventID, *desc, *category)

	case "list":
		app.list()

	case "show":
		if len(os.Args) < 3 {
			fmt.Println("show <event_id>")
			return
		}
		app.show(os.Args[2])

	case "done":
		if len(os.Args) < 4 {
			fmt.Println("done <event_id> <task_id>")
			return
		}
		app.markDone(os.Args[2], os.Args[3])

	case "delete":
		if len(os.Args) < 3 {
			fmt.Println("delete <event_id>")
			return
		}
		app.delete(os.Args[2])

	default:
		fmt.Println("Unknown command.")
	}
}
