# event_organizer.php
#!/usr/bin/env php
<?php

define('DATA_FILE', 'events.json');

class Task {
    public $id;
    public $description;
    public $category;
    public $completed;

    function __construct($description, $category = '') {
        $this->id = substr(bin2hex(random_bytes(4)), 0, 8);
        $this->description = $description;
        $this->category = $category;
        $this->completed = false;
    }

    function toArray() {
        return ['id' => $this->id, 'description' => $this->description,
                'category' => $this->category, 'completed' => $this->completed];
    }

    static function fromArray($data) {
        $task = new self($data['description'], $data['category']);
        $task->id = $data['id'];
        $task->completed = $data['completed'] ?? false;
        return $task;
    }
}

class Event {
    public $id;
    public $name;
    public $date;
    public $description;
    public $tasks = [];

    function __construct($name, $date = '', $description = '') {
        $this->id = substr(bin2hex(random_bytes(4)), 0, 8);
        $this->name = $name;
        $this->date = $date;
        $this->description = $description;
    }

    function toArray() {
        return [
            'id' => $this->id,
            'name' => $this->name,
            'date' => $this->date,
            'description' => $this->description,
            'tasks' => array_map(function($t) { return $t->toArray(); }, $this->tasks)
        ];
    }

    static function fromArray($data) {
        $ev = new self($data['name'], $data['date'], $data['description']);
        $ev->id = $data['id'];
        $ev->tasks = array_map(function($t) { return Task::fromArray($t); }, $data['tasks'] ?? []);
        return $ev;
    }
}

class Organizer {
    private $events = [];

    function __construct() {
        $this->load();
    }

    function load() {
        if (file_exists(DATA_FILE)) {
            $data = json_decode(file_get_contents(DATA_FILE), true);
            $this->events = array_map(function($e) { return Event::fromArray($e); }, $data);
        }
    }

    function save() {
        file_put_contents(DATA_FILE, json_encode(array_map(function($e) { return $e->toArray(); }, $this->events), JSON_PRETTY_PRINT));
    }

    function getEvent($id) {
        foreach ($this->events as $e) {
            if ($e->id == $id) return $e;
        }
        return null;
    }

    function create($name, $date = '', $description = '') {
        $ev = new Event($name, $date, $description);
        $this->events[] = $ev;
        $this->save();
        echo "✅ Event created: {$ev->name} (ID: {$ev->id})\n";
    }

    function addTask($eventId, $desc, $category = '') {
        $ev = $this->getEvent($eventId);
        if (!$ev) {
            echo "Event $eventId not found.\n";
            return;
        }
        $task = new Task($desc, $category);
        $ev->tasks[] = $task;
        $this->save();
        echo "✅ Task added: {$task->description} (ID: {$task->id})\n";
    }

    function list() {
        if (empty($this->events)) {
            echo "No events.\n";
            return;
        }
        echo "\n📋 Events:\n";
        foreach ($this->events as $e) {
            $total = count($e->tasks);
            $done = 0;
            foreach ($e->tasks as $t) if ($t->completed) $done++;
            $pct = $total > 0 ? round($done / $total * 100) : 0;
            echo "  {$e->name} (ID: {$e->id}) – {$done}/{$total} tasks done ({$pct}%)\n";
        }
    }

    function show($eventId) {
        $ev = $this->getEvent($eventId);
        if (!$ev) {
            echo "Event $eventId not found.\n";
            return;
        }
        $total = count($ev->tasks);
        $done = 0;
        foreach ($ev->tasks as $t) if ($t->completed) $done++;
        $pct = $total > 0 ? round($done / $total * 100) : 0;
        echo "\n📋 Event: {$ev->name}\n";
        if ($ev->date) echo "Date: {$ev->date}\n";
        if ($ev->description) echo "Description: {$ev->description}\n";
        echo "Progress: $done/$total tasks done ($pct%)\n";
        if ($total == 0) {
            echo "No tasks yet.\n";
            return;
        }
        echo "\nTasks:\n";
        foreach ($ev->tasks as $i => $task) {
            $status = $task->completed ? '[X]' : '[ ]';
            $cat = $task->category ? " ({$task->category})" : '';
            echo "  " . ($i+1) . ". $status {$task->description}$cat (ID: {$task->id})\n";
        }
    }

    function markDone($eventId, $taskId) {
        $ev = $this->getEvent($eventId);
        if (!$ev) {
            echo "Event $eventId not found.\n";
            return;
        }
        foreach ($ev->tasks as $task) {
            if ($task->id == $taskId) {
                if ($task->completed) {
                    echo "Task already completed.\n";
                } else {
                    $task->completed = true;
                    $this->save();
                    echo "✅ Task marked as done: {$task->description}\n";
                }
                return;
            }
        }
        echo "Task $taskId not found in this event.\n";
    }

    function delete($eventId) {
        foreach ($this->events as $i => $e) {
            if ($e->id == $eventId) {
                $name = $e->name;
                array_splice($this->events, $i, 1);
                $this->save();
                echo "✅ Event deleted: $name\n";
                return;
            }
        }
        echo "Event $eventId not found.\n";
    }
}

if ($argc < 2) {
    die("Usage: php event_organizer.php <command> [options]\n");
}
$app = new Organizer();
$cmd = $argv[1];

switch ($cmd) {
    case 'create':
        if ($argc < 3) die("create <name> [--date DATE] [--desc TEXT]\n");
        $name = $argv[2];
        $date = '';
        $desc = '';
        for ($i=3; $i<$argc; $i++) {
            if ($argv[$i] == '--date' && isset($argv[$i+1])) { $date = $argv[++$i]; }
            if ($argv[$i] == '--desc' && isset($argv[$i+1])) { $desc = $argv[++$i]; }
        }
        $app->create($name, $date, $desc);
        break;

    case 'add-task':
        if ($argc < 4) die("add-task <event_id> <description> [--category CAT]\n");
        $eventId = $argv[2];
        $desc = $argv[3];
        $category = '';
        for ($i=4; $i<$argc; $i++) {
            if ($argv[$i] == '--category' && isset($argv[$i+1])) { $category = $argv[++$i]; }
        }
        $app->addTask($eventId, $desc, $category);
        break;

    case 'list':
        $app->list();
        break;

    case 'show':
        if ($argc < 3) die("show <event_id>\n");
        $app->show($argv[2]);
        break;

    case 'done':
        if ($argc < 4) die("done <event_id> <task_id>\n");
        $app->markDone($argv[2], $argv[3]);
        break;

    case 'delete':
        if ($argc < 3) die("delete <event_id>\n");
        $app->delete($argv[2]);
        break;

    default:
        echo "Unknown command.\n";
}
?>
