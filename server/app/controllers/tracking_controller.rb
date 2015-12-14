require 'socket'

class TrackingController < ApplicationController
  def index

  end

  def update
    port = 1222
    s = TCPSocket.new('192.168.1.20', port)
    s.puts "w#,"

    @coord = {}
    @msg = 'Updated'
    info = nil
    while not info do
      info = s.gets
    end
    while info do
      cur, pos = info.split('#')
      if cur == 'm'
        pos.chomp!
        if !pos.empty?
          @msg = pos
        end
        @coord[cur] = @msg
      else
        x, y = pos.split(',')
        if  cur != 'c'
          cur = 'l' + cur 
        end
        @coord[cur] = [x.to_i, y.to_i]
      end
      info = s.gets
    end

    puts @coord.to_xml

    respond_to do |format|
      format.xml {render xml: @coord.to_xml}
    end
  end
end
