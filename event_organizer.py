# event_organizer.py
import json
import os
import sys
import argparse
import uuid
from datetime import datetime

DATA_FILE = "events.json"

class Task:
    def __init__(self, description, category="", completed=False, task_id=None):
        self.id = task_id or str(uuid.uuid4())[:8]
        self.description = description
        self.category = category
        self.completed = completed

    def to_dict(self):
        return {"id": self.id, "description": self.description,
                "category": self.category, "completed": self.completed}

    @classmethod
    def from_dict(cls, data):
        return cls(data["description"], data.get("category", ""),
                   data.get("completed", False), data.get("id"))

class Event:
    def __init__(self, name, date="", description="", event_id=None):
        self.id = event_id or str(uuid.uuid4())[:8]
        self.name = name
        self.date = date
        self.description = description
        self.tasks = []

    def to_dict(self):
        return {
            "id": self.id,
            "name": self.name,
            "date": self.date,
            "description": self.description,
            "tasks": [t.to_dict() for t in self.tasks]
        }

    @classmethod
    def from_dict(cls, data):
        ev = cls(data["name"], data.get("date", ""), data.get("description", ""), data.get("id"))
        ev.tasks = [Task.from_dict(t) for t in data.get("tasks", [])]
        return ev

class Organizer:
    def __init__(self):
        self.events = []
        self.load()

    def load(self):
        if os.path.exists(DATA_FILE):
            with open(DATA_FILE, "r") as f:
                data = json.load(f)
                self.events = [Event.from_dict(e) for e in data]

    def save(self):
        with open(DATA_FILE, "w") as f:
            json.dump([e.to_dict() for e in self.events], f, indent=2)

    def get_event(self, event_id):
        for e in self.events:
            if e.id == event_id:
                return e
        return None

    def create(self, name, date="", description=""):
        ev = Event(name, date, description)
        self.events.append(ev)
        self.save()
        print(f"✅ Event created: {ev.name} (ID: {ev.id})")

    def add_task(self, event_id, description, category=""):
        ev = self.get_event(event_id)
        if not ev:
            print(f"Event {event_id} not found.")
            return
        task = Task(description, category)
        ev.tasks.append(task)
        self.save()
        print(f"✅ Task added: {task.description} (ID: {task.id})")

    def list_events(self):
        if not self.events:
            print("No events.")
            return
        print("\n📋 Events:")
        for e in self.events:
            total = len(e.tasks)
            done = sum(1 for t in e.tasks if t.completed)
            pct = (done / total * 100) if total else 0
            print(f"  {e.name} (ID: {e.id}) – {done}/{total} tasks done ({pct:.0f}%)")

    def show(self, event_id):
        ev = self.get_event(event_id)
        if not ev:
            print(f"Event {event_id} not found.")
            return
        total = len(ev.tasks)
        done = sum(1 for t in ev.tasks if t.completed)
        pct = (done / total * 100) if total else 0
        print(f"\n📋 Event: {ev.name}")
        if ev.date:
            print(f"Date: {ev.date}")
        if ev.description:
            print(f"Description: {ev.description}")
        print(f"Progress: {done}/{total} tasks done ({pct:.0f}%)\n")
        if not ev.tasks:
            print("No tasks yet.")
            return
        print("Tasks:")
        for i, task in enumerate(ev.tasks, 1):
            status = "[X]" if task.completed else "[ ]"
            cat = f" ({task.category})" if task.category else ""
            print(f"  {i}. {status} {task.description}{cat} (ID: {task.id})")

    def mark_done(self, event_id, task_id):
        ev = self.get_event(event_id)
        if not ev:
            print(f"Event {event_id} not found.")
            return
        for task in ev.tasks:
            if task.id == task_id:
                if task.completed:
                    print(f"Task already completed.")
                else:
                    task.completed = True
                    self.save()
                    print(f"✅ Task marked as done: {task.description}")
                return
        print(f"Task {task_id} not found in this event.")

    def delete(self, event_id):
        ev = self.get_event(event_id)
        if not ev:
            print(f"Event {event_id} not found.")
            return
        self.events.remove(ev)
        self.save()
        print(f"✅ Event deleted: {ev.name}")

def main():
    parser = argparse.ArgumentParser(description="Event Organizer")
    subparsers = parser.add_subparsers(dest="cmd", required=True)

    create_parser = subparsers.add_parser("create")
    create_parser.add_argument("name")
    create_parser.add_argument("--date", help="YYYY-MM-DD")
    create_parser.add_argument("--desc", help="Description")

    add_task_parser = subparsers.add_parser("add-task")
    add_task_parser.add_argument("event_id")
    add_task_parser.add_argument("description")
    add_task_parser.add_argument("--category", default="")

    subparsers.add_parser("list")

    show_parser = subparsers.add_parser("show")
    show_parser.add_argument("event_id")

    done_parser = subparsers.add_parser("done")
    done_parser.add_argument("event_id")
    done_parser.add_argument("task_id")

    delete_parser = subparsers.add_parser("delete")
    delete_parser.add_argument("event_id")

    args = parser.parse_args()
    app = Organizer()

    if args.cmd == "create":
        app.create(args.name, args.date or "", args.desc or "")
    elif args.cmd == "add-task":
        app.add_task(args.event_id, args.description, args.category)
    elif args.cmd == "list":
        app.list_events()
    elif args.cmd == "show":
        app.show(args.event_id)
    elif args.cmd == "done":
        app.mark_done(args.event_id, args.task_id)
    elif args.cmd == "delete":
        app.delete(args.event_id)

if __name__ == "__main__":
    main()
