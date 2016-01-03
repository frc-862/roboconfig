require 'json'

def generate_field(k, v, base=[])
  if v.is_a?(Hash)
    result = %Q(<fieldset class="callout light-gray">\n)
    result += %Q(<h5>#{k}</h5>\n)
    new_base = [*base, k]
    v.each do |k,v|
      result += generate_field(k,v, new_base) + "\n"
    end
    result += %Q(</fieldset>\n)
  else
    nl = [*base, k]
    name = nl.first + nl[1..-1].map { |v| "[#{v}]" }.join
    %Q(<div class="row"><div class="large-12 columns"><label>#{k}</label><input type="text" name="#{name}:auto" value="#{v}" /></div></div>)
  end
end

json = JSON.parse(IO.read(ARGV.first || "sirius.json"))
json.each do |k,v|
  puts generate_field(k,v)
end

__END__
          <fieldset class="callout light-gray">
            <h5>Auton</h5>

          <div class="row">
            <div class="large-12 columns">
              <label>Throttle Ramp Rate</label>
              <input type="text" placeholder="0.05" />
            </div>
          </div>
