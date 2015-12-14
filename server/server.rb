require 'socket'
require 'set'

host = "192.168.1.20"
local_port = 1222
server = TCPServer.new(host, local_port)

class Sys202C
	attr_accessor :car_x, :car_y, :led_x, :led_y, :led_num, :car_msg

	def initialize
		@car_x = 140
		@car_y = 0
		@led_x = [0, 60.2, 57.4, 6.0, -80.1, -76.5]
		@led_y = [0, 59.3, -60.5, -5.3, 55.2, -63.2]
		@led_num = [1, 2, 3, 4, 5]
		@car_msg = ''
	end

	def receive_info(client)
		puts "Retrieving info..."
		info = ""
		while info.empty?
			info = client.gets.chomp!
		end
		puts "Info retrieved: #{info}. Parsing..."
		infos = info.split('#')
		cur = infos[0]
		if cur == "w"
			puts "Received web requests."
		else
			x, y = infos[1].split(",")
			if cur == "c"
				self.car_x = x.to_f
				self.car_y = y.to_f
				puts "Car position updated to (#{self.car_x}, #{self.car_y})."
				if infos.length > 2
					self.car_msg += infos[2]
					puts "Received info: #{infos[2]}"
				end
			#else
				#puts "LED #{cur}:"
				#self.led_num.add(cur)
				#self.led_x[cur] = x.to_f
				#self.led_y[cur] = y.to_f
				#puts "Position updated to (#{self.led_x[cur]}, #{self.led_y[cur]})."
			end
		end
		return cur
	end

	def send_instr(car)
		cur = ""
		self.led_num.each do |l|
			cur += l.to_s
			cur += '#'
			cur += self.led_x[l].to_s
			cur += ','
			cur += self.led_y[l].to_s
			cur += "\n"
		end
		car.puts cur
	end

	def send_all(web)
		web.puts 'c#' + self.car_x.to_s + ',' + self.car_y.to_s

		self.led_num.each do |l|
			cur = l.to_s
			cur += '#'
			cur += self.led_x[l].to_s
			cur += ','
			cur += self.led_y[l].to_s
			web.puts cur
		end

		web.puts 'm#' + self.car_msg
	end
end

sys = Sys202C.new

loop do
	Thread.start(server.accept) do |client|
		puts "Accepted connection from a client."
		obj = sys.receive_info(client)
		if obj == "c"
			sys.send_instr(client)
		elsif obj == "w"
			sys.send_all(client)
		end
		client.close
	end
end
