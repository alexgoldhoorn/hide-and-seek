def calcTriExpRewMapSize(rows,cols):
	maxSteps = 2*(rows+cols)
	Rmax = rows*cols
	return (maxSteps,Rmax,calcTriExpRew(maxSteps,Rmax))



def calcTriExpRew(d,Rmax):
	g=1
	Rtot = 0
	for i in range(d):
		Rtot=Rtot+g * (Rmax - (d-i))
		g=g*0.95
	return Rtot


def calcExpRew2(d,Rmax):
	g=1
	Rtot = 0
	for i in range(d):
		Rtot=Rtot+g * (Rmax )
		g=g*0.95
	return Rtot





Todo


- check use of node in tree that has already been created
- do rollout not with random but maximum score action
- why all belief points inconsistent after update..??
- maybe put random factor in added value to value obtained by expected reward
