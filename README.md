📋 Event Organizer (Checklists) — Multi‑Language Task Manager for Events
8 languages, one complete event planning tool – create events, manage checklists, track progress, and never forget a task again – right from your terminal.

✨ Features
🎯 Create events – name, date, description

✅ Add tasks to an event’s checklist with optional categories

☑️ Mark tasks as done – track what’s completed

📊 View progress – see percentage and status for each event

📋 List all events – with completion stats

🔍 Show event details – full checklist with task status

💾 Persistent storage – all data saved in events.json

🗑️ Delete events – remove when finished

🧰 Supported Languages & Files
Language	File	Dependencies
Python	event_organizer.py	none (stdlib)
Go	event_organizer.go	none (stdlib)
JavaScript (Node)	event_organizer.js	commander (optional)
Ruby	event_organizer.rb	json, date
PHP	event_organizer.php	none (extensions)
Java	EventOrganizer.java	Java 8+
C#	EventOrganizer.cs	.NET Core 3.1+
C++	event_organizer.cpp	nlohmann/json
🚀 Quick Start
All implementations follow the same CLI pattern:

bash
# Create a new event
<command> create "Birthday Party" --date "2026-12-25" --desc "Christmas celebration"

# Add a task to an event (use event ID from list)
<command> add-task <event_id> "Buy cake" --category "Food"

# List all events
<command> list

# Show event details with checklist
<command> show <event_id>

# Mark a task as completed (use task ID from show)
<command> done <event_id> <task_id>

# Delete an event
<command> delete <event_id>
Commands:

create <name> [--date DATE] [--desc TEXT] – create event

add-task <event_id> <task_description> [--category CAT] – add task

list – show all events

show <event_id> – show event with tasks

done <event_id> <task_id> – mark task complete

delete <event_id> – delete event

📸 Example Output
text
📋 Events:
1. Birthday Party (ID: abc123) – 2/5 tasks done (40%)
2. Conference (ID: def456) – 0/3 tasks done (0%)

📋 Event: Birthday Party
Date: 2026-12-25
Description: Christmas celebration
Progress: 2/5 tasks done (40%)

Tasks:
  1. [X] Buy cake (Food)
  2. [ ] Invite guests (Logistics)
  3. [ ] Decorate venue (Decor)
  4. [X] Prepare music (Entertainment)
  5. [ ] Order food (Food)
📁 Repository Structure
text
.
├── README.md
├── python/
│   └── event_organizer.py
├── go/
│   └── event_organizer.go
├── javascript/
│   └── event_organizer.js
├── ruby/
│   └── event_organizer.rb
├── php/
│   └── event_organizer.php
├── java/
│   └── EventOrganizer.java
├── csharp/
│   └── EventOrganizer.cs
└── cpp/
    └── event_organizer.cpp
