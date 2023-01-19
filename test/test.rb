require 'test/unit'
require 'swe4r'

class Swe4rTest < Test::Unit::TestCase
  
  def test_swe_set_ephe_path
    assert_equal(nil, Swe4r::swe_set_ephe_path('path'))
  end
  
  def test_swe_julday
    assert_equal(2444838.972916667, Swe4r::swe_julday(1981, 8, 22, 11.35))
  end
  
  def test_swe_set_topo
    assert_equal(nil, Swe4r::swe_set_topo(-112.183333, 45.45, 1524))
  end 
  
  def test_swe_set_sid_mode
    assert_equal(nil, Swe4r::swe_set_sid_mode(Swe4r::SE_SIDM_LAHIRI, 0, 0)) # Use Lahiri mode
    assert_equal(nil, Swe4r::swe_set_sid_mode(Swe4r::SE_SIDM_USER, 2415020.5, 22.460489112721632)) # Use user defined mode
  end
  
  def test_swe_get_ayanamsa_ut
    
    # Test using default sidereal mode
    Swe4r::swe_set_sid_mode(Swe4r::SE_SIDM_FAGAN_BRADLEY, 0, 0)
    assert_equal(24.483840294903757, Swe4r::swe_get_ayanamsa_ut(2444838.972916667))
    
    # Test using Lahari sidereal mode
    Swe4r::swe_set_sid_mode(Swe4r::SE_SIDM_LAHIRI, 0, 0)
    assert_equal(23.600632656944185, Swe4r::swe_get_ayanamsa_ut(2444838.972916667))
    
    # Test using user defined sidereal mode
    Swe4r::swe_set_sid_mode(Swe4r::SE_SIDM_USER, 2415020.5, 22.460489112721632)
    assert_equal(23.600591306635067, Swe4r::swe_get_ayanamsa_ut(2444838.972916667))
    
  end
  
  def test_swe_calc_ut
    
    # The Moshier Ephemeris will be used for all tests since it does not require ephemeris files
    
    # Test #1...
    # Body: Sun
    # Flags: Moshier Ephemeris
    body = Swe4r::swe_calc_ut(2444838.972916667, Swe4r::SE_SUN, Swe4r::SEFLG_MOSEPH)
    assert_equal(149.26566155075085, body[0])
    assert_equal(-0.00012095608841323021, body[1])
    assert_equal(1.0112944920684557, body[2])
    assert_equal(0.0, body[3])
    assert_equal(0.0, body[4])
    assert_equal(0.0, body[5])
    
    # Test #2...
    # Body: Sun
    # Flags: Moshier Ephemeris, High Precision Speed
    body = Swe4r::swe_calc_ut(2444838.972916667, Swe4r::SE_SUN, Swe4r::SEFLG_MOSEPH|Swe4r::SEFLG_SPEED)
    assert_equal(149.26566155075085, body[0])
    assert_equal(-0.0001209560884116579, body[1])
    assert_equal(1.0112944920684555, body[2])
    assert_equal(0.9636052090727139, body[3])
    assert_equal(1.3573058519899091e-05, body[4])
    assert_equal(-0.0002028500183236368, body[5])
    
     # Test #3...
     # Body: Sun
     # Flags: Moshier Ephemeris, High Precision Speed, True Positions
     body = Swe4r::swe_calc_ut(2444838.972916667, Swe4r::SE_SUN, Swe4r::SEFLG_MOSEPH|Swe4r::SEFLG_TRUEPOS|Swe4r::SEFLG_SPEED)
     assert_equal(149.27128949344328, body[0])
     assert_equal(-0.00012086030263409888, body[1])
     assert_equal(1.0112944920684555, body[2])
     assert_equal(0.9636063426569487, body[3])
     assert_equal(1.3573775684911814e-05, body[4])
     assert_equal(-0.00020285001901355993, body[5])
     
     # Test #4...
     # Body: Sun
     # Flags: Moshier Ephemeris, High Precision Speed, True Positions, Topocentric
     Swe4r::swe_set_topo(-112.183333, 45.45, 1524)
     body = Swe4r::swe_calc_ut(2444838.972916667, Swe4r::SE_SUN, Swe4r::SEFLG_MOSEPH|Swe4r::SEFLG_TRUEPOS|Swe4r::SEFLG_SPEED|Swe4r::SEFLG_TOPOCTR)
     assert_equal(149.27327994957028, body[0])
     assert_equal(-0.0013677823223172442, body[1])
     assert_equal(1.0113041763512773, body[2])
     assert_equal(0.9683657793763634, body[3])
     assert_equal(0.0037440250636134086, body[4])
     assert_equal(-0.0003579187534546972, body[5])
    
     # Test #5...
     # Body: Sun
     # Flags: Moshier Ephemeris, High Precision Speed, True Positions, Topocentric, Sidereal (Lahiri Mode)
     Swe4r::swe_set_topo(-112.183333, 45.45, 1524)
     Swe4r::swe_set_sid_mode(1, 0, 0)
     body = Swe4r::swe_calc_ut(2444838.972916667, Swe4r::SE_SUN, Swe4r::SEFLG_MOSEPH|Swe4r::SEFLG_TRUEPOS|Swe4r::SEFLG_SPEED|Swe4r::SEFLG_TOPOCTR|Swe4r::SEFLG_SIDEREAL)
     assert_equal(125.67660487807687, body[0])
     assert_equal(-0.0013677823223169163, body[1])
     assert_equal(1.0113041763512773, body[2])
     assert_equal(0.9683250913810203, body[3])
     assert_equal(0.0037468508076796666, body[4])
     assert_equal(-0.0003579187527254481, body[5])
    
  end
  
  def test_swe_houses
    
    # Test each house system
    systems = ['P','K','O','R','C','A','E','V','X','H','T','B'] # 'G'
    systems.each do |s|
      Swe4r::swe_houses(2444838.972916667, 45.45, -112.183333, s)
      puts s
    end
    
    # Test using Placidus house system
    assert_equal(
      [[0.0,
        133.95429950225963,
        153.80292074191388,
        178.80796487675514,
        210.87358004433614,
        248.4391877068773,
        284.26552526926207,
        313.95429950225963,
        333.8029207419139,
        358.80796487675514,
        30.873580044336133,
        68.43918770687732,
        104.26552526926204],
       [133.95429950225963,
        30.873580044336133,
        28.745753308674352,
        273.2404152103502,
        116.71405283378715,
        92.42567887228608,
        133.52799512863683,
        272.4256788722861,
        0.0,
        0.0]],
    Swe4r::swe_houses(2444838.972916667, 45.45, -112.183333, 'P'))
  end
  
end

