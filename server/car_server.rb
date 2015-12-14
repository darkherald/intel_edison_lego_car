require 'socket'

port = 731
server = TCPServer.open(port)

def record_pos(led_num, x, y)
	puts "LED \##{led_num}"
	puts "(#{x}, #{y})"
end

loop do
	client = server.accept
	while info = client.gets
		cur, info = info.split('#')
		puts "LED \##{cur}: (#{info})"
	end
	client.close
end
