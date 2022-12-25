require 'swe4r'
require 'ostruct'

class SwissEphemeris
    BODIES = {
        SO: { id: Swe4r::SE_SUN, name: 'sun', symbol: '☉'},
        MO: { id: Swe4r::SE_MOON, name: 'moon', symbol: '☽'},
        ME: { id: Swe4r::SE_MERCURY, name: 'mercury', symbol: '☿'},
        VE: { id: Swe4r::SE_VENUS, name: 'venus', symbol: '♀'},
        MA: { id: Swe4r::SE_MARS, name: 'mars', symbol: '♂'},
        JU: Swe4r::SE_JUPITER,
        SA: Swe4r::SE_SATURN,
        UR: Swe4r::SE_URANUS,
        NE: Swe4r::SE_NEPTUNE,
        PL: Swe4r::SE_PLUTO,
        MN: Swe4r::SE_MEAN_NODE,
        TN: Swe4r::SE_TRUE_NODE,
        LI: Swe4r::SE_MEAN_APOG,
        TL: Swe4r::SE_OSCU_APOG, 
        EA: Swe4r::SE_EARTH,
        CH: Swe4r::SE_CHIRON,
        CE: Swe4r::SE_CERES,
        PA: Swe4r::SE_PALLAS,
        PH: Swe4r::SE_PHOLUS,
        JN: Swe4r::SE_JUNO,
        VS: Swe4r::SE_VESTA,
        CU: Swe4r::SE_CUPIDO,
        HA: Swe4r::SE_HADES,
        ZE: Swe4r::SE_ZEUS,
        KR: Swe4r::SE_KRONOS,
        AP: Swe4r::SE_APOLLON,
        AD: Swe4r::SE_ADMETOS,
        VU: Swe4r::SE_VULKANUS,
        PO: Swe4r::SE_POSEIDON,    
    }

    SIGNS = ["Ari", "Tau", "Gem", "Can", "Leo", "Vir", "Lib", "Sco", "Sag", "Cap", "Aqu", "Pis"]
    
    def path= path
      Swe4r::swe_set_ephe_path(path.to_s)
    end

    def jpl_file= path
      swe_set_jpl_file(path.to_s);
    end


    class Calculator
        attr_accessor :latitude, :longitude, :altitude, :date
        attr_reader :flags, :jd, :cusps, :asc, :mc, :armc, :vertex
        def initialize params={}
            # https://www.astro.com/swisseph/swephprg.htm
            params[:use_moshier_ephemeris] = true unless params[:moshier_ephemeris] == false
            @flags = Swe4r::SEFLG_SPEED  
            @flags |= Swe4r::SEFLG_TRUEPOS if params[:true_positions] # no light time correction - return true positions, not apparent
            @flags |= Swe4r::SEFLG_SIDEREAL if params[:sidereal]
            @flags |= Swe4r::SEFLG_TOPOCTR if params[:topocentric]
            @flags |= Swe4r::SEFLG_HELCTR if params[:heliocentric]
            @flags |= Swe4r::SEFLG_MOSEPH if params[:use_moshier_ephemeris]
            @flags |= Swe4r::SEFLG_JPLEPH if params[:use_jpl_ephemeris]
            @flags |= Swe4r::SEFLG_SWIEPH if params[:use_swiss_ephemeris]
            @flags |= Swe4r::SEFLG_EQUATORIAL if params[:equatorial] # right ascension and declination instead of lat/lon
            return self
        end

        def date= time
            @jd = Swe4r::swe_julday(time.year, time.month, time.day, time.hour + time.min/60.0)
        end

        def set_topo latitude, longitude, altitude = 0
            @latitude = latitude
            @longitude = longitude
            Swe4r::swe_set_topo(longitude, latitude, altitude)
        end

        def position( object )
            obj = SwissEphemeris::BODIES[object]
            values = Swe4r::swe_calc_ut( self.jd, obj[:id], self.flags )
            # [longitude, latitude, distance, speed in long., speed in lat., and speed in dist.]
            return Body.new( values[0], values[1], values[2], values[3], obj[:name], obj[:symbol] )
        end

        def houses( method = 'K' )
            output = Swe4r::swe_houses( self.jd, self.latitude, self.longitude, method )
            @cusps = output[1..12];
            ascmc = output[13..-1];
            @asc, @mc, @armc, @vertex = @ascm[0..3]
        end
    end

    class Body
        attr_reader :degree, :lat, :lon, :distance, :velocity, :name, :symbol
        def initialize( lon, lat, distance, velocity, name, symbol )
            @lat, @lon, @distance, @velocity, @name, @symbol = lat, lon, distance, velocity, name, symbol
            @sign, @degree = lon.divmod(30)
            degree = @degree.floor
            @minute = (@degree-degree) * 60.0;
            @minute = @minute.round
            @degree = degree
        end

        def sign
          SIGNS[@sign]
        end

        def to_s( kind = :full )
          pos = "#{@degree}º #{sign} #{@minute}"
          case kind
          when :short
            return pos
          when :full
            return "#{name.capitalize} #{pos}"
          end
        end


    end
end


calculator = SwissEphemeris::Calculator.new(light_correction: false, topocentric: true)

calculator.date = Time.new(1979, 5, 8, 10, 57, 0, "-09:00")

require 'geocoder'
results = Geocoder.search('Anchorage, AK')
calculator.set_topo( results.first.longitude, results.first.latitude )

sun = calculator.position(:SO)
moon = calculator.position(:MO)
mercury = calculator.position(:ME)


# moon_position = calculator.position(:moon)
# mercury_position = calculator.position(:mercury)
# venus_position = calculator.position(:venus)
# mars_position = calculator.position(:mars)
# jupiter_position = calculator.position(:jupiter)
# saturn_position = calculator.position(:saturn)
# uranus_position = calculator.position(:uranus)
# neptune_position = calculator.position(:neptune)
# pluto_position = calculator.position(:pluto)

# chiron_position = calculator.position(:chiron)
# pallas_position = calculator.position(:pallas)
# ceres_position = calculator.position(:ceres)
# vesta_position = calculator.position(:vesta)
# Juno_position = calculator.position(:juno)



#
SYMBOLS = {
    SO: '☉',
    MO: '☽',
    ME: '☿',
    VE: '♀',
    MA: '♂',
    JU: '♃',
    SA: '♄',
    UR: '♅',
    NE: '♆',
    PL: '⛢', #'♇',
    CH: '',
    # PA:
    # CE:
    # VS:
    # JO:
}


def draw_chart( planets )
    # First, create a new SVG document:
    svg = RSVG::Handle.new

    # Set the width and height of the chart:
    svg.width = 800
    svg.height = 800

    # Set the background color to white:
    svg.add_rect(0, 0, svg.width, svg.height, fill: 'white')

    # Add the positions of the planets and asteroids to the chart:

    # Set the radius of the chart in pixels:
    radius = 300

    # Calculate the x and y coordinates for each planet and asteroid based on its position and the radius of the chart:
    sun_position = planets[:sun]
    sun_x = radius * Math.cos(sun_position * Math::PI / 180)
    sun_y = radius * Math.sin(sun_position * Math::PI / 180)
    moon_position = planets[:moon]
    moon_x = radius * Math.cos(moon_position * Math::PI / 180)
    moon_y = radius * Math.sin(moon_position * Math::PI / 180)
    # ...

    # Add a circle for each planet and asteroid to the chart:
    svg.add_text(sun_x, sun_y, SYMBOLS[:SO], fill: 'yellow')
    # svg.add_text(sun_x + 10, sun_y + 10, "Sun")
    svg.add_text(moon_x, moon_y, SYMBOLS[:SO], fill: 'gray')

end


