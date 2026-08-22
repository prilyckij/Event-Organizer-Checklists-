# event_organizer.rb
#!/usr/bin/env ruby
require 'json'
require 'securerandom'

DATA_FILE = 'events.json'

class Task
  attr_accessor :id, :description, :category, :completed

  def initialize(description, category = '')
    @id = SecureRandom.hex(4)
    @description = description
    @category = category
    @completed = false
  end

  def to_hash
    { id: @id, description: @description, category: @category, completed: @completed }
  end

  def self.from_hash(h)
    task = new(h['description'], h['category'])
    task.id = h['id']
    task.completed = h['completed'] || false
    task
  end
end

class Event
  attr_accessor :id, :name, :date, :description, :tasks

  def initialize(name, date = '', description = '')
    @id = SecureRandom.hex(4)
    @name = name
    @date = date
    @description = description
    @tasks = []
  end

  def to_hash
    { id: @id, name: @name, date: @date, description: @description, tasks: @tasks.map(&:to_hash) }
  end

  def self.from_hash(h)
    ev = new(h['name'], h['date'], h['description'])
    ev.id = h['id']
    ev.tasks = h['tasks'].map { |t| Task.from_hash(t) }
    ev
  end
end

class Organizer
  attr_reader :events

  def initialize
    @events = []
    load
  end

  def load
    if File.exist?(DATA_FILE)
      data = JSON.parse(File.read(DATA_FILE))
      @events = data.map { |h| Event.from_hash(h) }
    end
  end

  def save
    File.write(DATA_FILE, JSON.pretty_generate(@events.map(&:to_hash)))
  end

  def get_event(id)
    @events.find { |e| e.id == id }
  end

  def create(name, date = '', description = '')
    ev = Event.new(name, date, description)
    @events << ev
    save
    puts "✅ Event created: #{ev.name} (ID: #{ev.id})"
  end

  def add_task(event_id, description, category = '')
    ev = get_event(event_id)
    unless ev
      puts "Event #{event_id} not found."
      return
    end
    task = Task.new(description, category)
    ev.tasks << task
    save
    puts "✅ Task added: #{task.description} (ID: #{task.id})"
  end

  def list
    if @events.empty?
      puts "No events."
      return
    end
    puts "\n📋 Events:"
    @events.each do |e|
      total = e.tasks.size
      done = e.tasks.count(&:completed)
      pct = total > 0 ? (done.to_f / total * 100).to_i : 0
      puts "  #{e.name} (ID: #{e.id}) – #{done}/#{total} tasks done (#{pct}%)"
    end
  end

  def show(event_id)
    ev = get_event(event_id)
    unless ev
      puts "Event #{event_id} not found."
      return
    end
    total = ev.tasks.size
    done = ev.tasks.count(&:completed)
    pct = total > 0 ? (done.to_f / total * 100).to_i : 0
    puts "\n📋 Event: #{ev.name}"
    puts "Date: #{ev.date}" unless ev.date.empty?
    puts "Description: #{ev.description}" unless ev.description.empty?
    puts "Progress: #{done}/#{total} tasks done (#{pct}%)"
    if total == 0
      puts "No tasks yet."
      return
    end
    puts "\nTasks:"
    ev.tasks.each_with_index do |task, i|
      status = task.completed ? '[X]' : '[ ]'
      cat = task.category.empty? ? '' : " (#{task.category})"
      puts "  #{i+1}. #{status} #{task.description}#{cat} (ID: #{task.id})"
    end
  end

  def mark_done(event_id, task_id)
    ev = get_event(event_id)
    unless ev
      puts "Event #{event_id} not found."
      return
    end
    task = ev.tasks.find { |t| t.id == task_id }
    unless task
      puts "Task #{task_id} not found in this event."
      return
    end
    if task.completed
      puts "Task already completed."
    else
      task.completed = true
      save
      puts "✅ Task marked as done: #{task.description}"
    end
  end

  def delete(event_id)
    ev = get_event(event_id)
    unless ev
      puts "Event #{event_id} not found."
      return
    end
    name = ev.name
    @events.delete(ev)
    save
    puts "✅ Event deleted: #{name}"
  end
end

if ARGV.empty?
  puts "Usage: event_organizer.rb <command> [options]"
  exit
end

app = Organizer.new
cmd = ARGV.shift

case cmd
when 'create'
  name = ARGV.shift
  date = ''
  desc = ''
  if ARGV.include?('--date')
    idx = ARGV.index('--date')
    date = ARGV[idx+1] if idx
  end
  if ARGV.include?('--desc')
    idx = ARGV.index('--desc')
    desc = ARGV[idx+1] if idx
  end
  app.create(name, date, desc)

when 'add-task'
  event_id = ARGV.shift
  desc = ARGV.shift
  category = ''
  if ARGV.include?('--category')
    idx = ARGV.index('--category')
    category = ARGV[idx+1] if idx
  end
  app.add_task(event_id, desc, category)

when 'list'
  app.list

when 'show'
  event_id = ARGV.shift
  app.show(event_id)

when 'done'
  event_id = ARGV.shift
  task_id = ARGV.shift
  app.mark_done(event_id, task_id)

when 'delete'
  event_id = ARGV.shift
  app.delete(event_id)

else
  puts "Unknown command."
end
