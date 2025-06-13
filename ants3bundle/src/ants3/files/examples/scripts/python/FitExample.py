import math

RealFrequency = 2.5
RealPhase = 1

bestD2 = 1e30

msg.showWindow()
msg.setGeometry([0,0, 500,600, False])
msg.clear()

def Functor(freq, phase) :
   global bestD2
   d2 = 0
   for i in range (0, 50) :
      trueData = math.sin(RealFrequency * i*math.pi/50 + RealPhase)
      testData = math.sin(freq  * i*math.pi/50 + phase)
      delta = trueData - testData
      d2 += delta * delta
   d2 = math.sqrt(d2)
   
   if d2 < bestD2 :
      bestD2 = d2
      s = " Frequency: " + core.str(freq, 7) + "   \tPhase: " + core.str(phase, 7) +"  \tDelta: " + core.str(bestD2, 7)
      msg.appendHtml(s)
   return d2
       
test = Functor(1.0, 0.5) #just to test the Functor explicitly and report syntax errors if any

mini.clear()
mini.setFunctorName("Functor")
mini.setMigrad() # recommended to use mini.setSimplex() when the cost function is computed based on the simulation results (affected by statistical fluctuations)
mini.addVariable("fr", 1.0, 0.2)
mini.addVariable("ph", 0.5, 0.2)
success = mini.run()

res = mini.getResults()

if success : print("Success!")
else : print("Fail!")

print("Found frequency of " + core.str(res[0], 7) + " and phase of " + core.str(res[1], 7))

