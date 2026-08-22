// event_organizer.js
#!/usr/bin/env node
const fs = require('fs');
const { program } = require('commander');
const { v4: uuidv4 } = require('uuid');

const DATA_FILE = 'events.json';

class Task {
    constructor(description, category = '') {
        this.id = uuidv4().slice(0,8);
        this.description = description;
        this.category = category;
        this.completed = false;
    }
}

class Event {
    constructor(name, date = '', description = '') {
        this.id = uuidv4().slice(0,8);
        this.name = name;
        this.date = date;
        this.description = description;
        this.tasks = [];
    }
}

class Organizer {
    constructor() {
        this.events = [];
        this.load();
    }

    load() {
        if (fs.existsSync(DATA_FILE)) {
            try {
                const data = JSON.parse(fs.readFileSync(DATA_FILE));
                this.events = data.map(e => {
                    const ev = new Event(e.name, e.date, e.description);
                    ev.id = e.id;
                    ev.tasks = e.tasks.map(t => {
                        const task = new Task(t.description, t.category);
                        task.id = t.id;
                        task.completed = t.completed || false;
                        return task;
                    });
                    return ev;
                });
            } catch (e) {}
        }
    }

    save() {
        fs.writeFileSync(DATA_FILE, JSON.stringify(this.events, null, 2));
    }

    getEvent(id) {
        return this.events.find(e => e.id === id);
    }

    create(name, date, desc) {
        const ev = new Event(name, date, desc);
        this.events.push(ev);
        this.save();
        console.log(`✅ Event created: ${ev.name} (ID: ${ev.id})`);
    }

    addTask(eventId, description, category) {
        const ev = this.getEvent(eventId);
        if (!ev) {
            console.log(`Event ${eventId} not found.`);
            return;
        }
        const task = new Task(description, category);
        ev.tasks.push(task);
        this.save();
        console.log(`✅ Task added: ${task.description} (ID: ${task.id})`);
    }

    list() {
        if (this.events.length === 0) {
            console.log('No events.');
            return;
        }
        console.log('\n📋 Events:');
        for (const e of this.events) {
            const total = e.tasks.length;
            const done = e.tasks.filter(t => t.completed).length;
            const pct = total ? (done / total * 100).toFixed(0) : 0;
            console.log(`  ${e.name} (ID: ${e.id}) – ${done}/${total} tasks done (${pct}%)`);
        }
    }

    show(eventId) {
        const ev = this.getEvent(eventId);
        if (!ev) {
            console.log(`Event ${eventId} not found.`);
            return;
        }
        const total = ev.tasks.length;
        const done = ev.tasks.filter(t => t.completed).length;
        const pct = total ? (done / total * 100).toFixed(0) : 0;
        console.log(`\n📋 Event: ${ev.name}`);
        if (ev.date) console.log(`Date: ${ev.date}`);
        if (ev.description) console.log(`Description: ${ev.description}`);
        console.log(`Progress: ${done}/${total} tasks done (${pct}%)`);
        if (total === 0) {
            console.log('No tasks yet.');
            return;
        }
        console.log('\nTasks:');
        ev.tasks.forEach((task, i) => {
            const status = task.completed ? '[X]' : '[ ]';
            const cat = task.category ? ` (${task.category})` : '';
            console.log(`  ${i+1}. ${status} ${task.description}${cat} (ID: ${task.id})`);
        });
    }

    markDone(eventId, taskId) {
        const ev = this.getEvent(eventId);
        if (!ev) {
            console.log(`Event ${eventId} not found.`);
            return;
        }
        const task = ev.tasks.find(t => t.id === taskId);
        if (!task) {
            console.log(`Task ${taskId} not found in this event.`);
            return;
        }
        if (task.completed) {
            console.log('Task already completed.');
        } else {
            task.completed = true;
            this.save();
            console.log(`✅ Task marked as done: ${task.description}`);
        }
    }

    delete(eventId) {
        const idx = this.events.findIndex(e => e.id === eventId);
        if (idx === -1) {
            console.log(`Event ${eventId} not found.`);
            return;
        }
        const name = this.events[idx].name;
        this.events.splice(idx, 1);
        this.save();
        console.log(`✅ Event deleted: ${name}`);
    }
}

program
    .command('create <name>')
    .option('--date <date>', 'Event date')
    .option('--desc <description>', 'Event description')
    .action((name, options) => {
        const app = new Organizer();
        app.create(name, options.date || '', options.desc || '');
    });

program
    .command('add-task <eventId> <description>')
    .option('--category <category>', 'Task category')
    .action((eventId, description, options) => {
        const app = new Organizer();
        app.addTask(eventId, description, options.category || '');
    });

program
    .command('list')
    .action(() => {
        const app = new Organizer();
        app.list();
    });

program
    .command('show <eventId>')
    .action((eventId) => {
        const app = new Organizer();
        app.show(eventId);
    });

program
    .command('done <eventId> <taskId>')
    .action((eventId, taskId) => {
        const app = new Organizer();
        app.markDone(eventId, taskId);
    });

program
    .command('delete <eventId>')
    .action((eventId) => {
        const app = new Organizer();
        app.delete(eventId);
    });

program.parse(process.argv);
