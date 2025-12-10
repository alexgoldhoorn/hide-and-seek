# Hide-and-Seek POMCP Simulator

Simulation framework for robot search and tracking using POMCP (Partially Observable Monte-Carlo Planning) and particle filters. Developed for PhD research on probabilistic human tracking in urban environments.

[![License: LGPL v3](https://img.shields.io/badge/License-LGPL%20v3-blue.svg)](https://www.gnu.org/licenses/lgpl-3.0)
[![Language: C++11](https://img.shields.io/badge/C++-11-00599C.svg)](https://isocpp.org/)
[![Platform: Linux](https://img.shields.io/badge/Platform-Linux-lightgrey.svg)](https://www.linux.org/)

> **Try it online:** [Interactive Hide-and-Seek Belief Simulation](https://alex.goldhoorn.net/projects/hide-and-seek-belief/)

## What's Included

- **POMCP & MOMDP solvers** for real-time robot decision making under uncertainty
- **Particle Filter & Kalman Filter** implementations for tracking
- **Multi-robot cooperative search** algorithms
- **Server-client architecture** for multiplayer games (humans vs robots)
- **ROS integration** for real robot experiments
- **MySQL logging** for experiment analysis
- Maps, policies, and test scenarios from published research

## Publications

**PhD Thesis (2017):**
> *"Searching and Tracking of Humans in Urban Environments by Humanoid Robots"*
> Alex Goldhoorn, IRI CSIC-UPC, Barcelona
> [PDF](https://alex.goldhoorn.net/thesis/Goldhoorn2017_PhD_thesis.pdf) | [Website](https://alex.goldhoorn.net/thesis/)

**Selected Papers:**
- [Searching and Tracking People with Cooperative Mobile Robots](https://alex.goldhoorn.net/publications/ar2016/) (Autonomous Robots, 2017)
- [Searching in Urban Environments with Dynamic Obstacles](https://alex.goldhoorn.net/publications/ras2016/) (Robotics and Autonomous Systems, 2017)
- [Continuous Real Time POMCP to Find-and-Follow People](https://alex.goldhoorn.net/publications/find-and-follow/) (Humanoids, 2014)

**[All publications](https://alex.goldhoorn.net/publications/)**

## Citation

If you use this code in your research, please cite:

```bibtex
@phdthesis{Goldhoorn2017,
    author = "Alex Goldhoorn",
    title = "Searching and Tracking of Humans in Urban Environments by Humanoid Robots",
    school = "Institut de Robòtica i Informàtica Industrial, CSIC-UPC",
    year = "2017",
    month = "June",
    address = "Barcelona, Spain"
}
```

## Quick Start

```bash
# Build the library
cd trunk/build
cmake ..
make

# Or build with Qt for GUI/server
cd trunk
qmake hsmomdp.pro && make    # Automated seeker
qmake hsserver.pro && make   # Game server
qmake hsclient.pro && make   # GUI client
```

See `trunk/README.txt` for detailed build instructions and command-line options.

## Dependencies

**Required:**
- g++ 4.8+ (C++11 support)
- Qt 5
- OpenCV 2.3+ (OpenCV 3.x/4.x supported with modifications)
- Eigen v3
- MySQL 5.6+ (optional, for logging)
- **iriutils** - IRI robotics library ([SVN](https://devel.iri.upc.edu/labrobotica/drivers/iriutils/trunk/), may require IRI access)

> **⚠️ Historical Code Notice:**
> This code is from 2011-2016. Dependencies are outdated by modern standards. Expect to update OpenCV API calls and use newer compiler versions. For a modern demo, see the [web-based simulation](https://alex.goldhoorn.net/projects/hide-and-seek-belief/).

## Example Usage

```bash
# Start game server
./bin/hsserver

# Run POMCP seeker on map 1
./bin/hsmomdp -Gh -scd -A 1 -u TestUser

# Flags: -Gh (hide-and-seek mode), -scd (POMCP discrete states)
# See trunk/README.txt or run with -h for all options
```

## Available Solvers

- **POMCP** (`-scd`, `-scc`) - Online Monte-Carlo planning (discrete/continuous)
- **MOMDP** (`-sl`) - Offline mixed-observability planning
- **Particle Filter** (`-sPu`) - Particle-based tracking
- **Kalman Filter** (`-sKr`) - Linear tracking
- **Multi-robot** (`-sMc`) - Cooperative search
- **Smart Seeker** (`-ss`) - Heuristic baseline

See `trunk/README.txt` for complete command reference.

## License

Copyright (C) 2011-2016 Institut de Robòtica i Informàtica Industrial, CSIC-UPC.

This software is free software: you can redistribute it and/or modify it under the terms of the **GNU Lesser General Public License** as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this program. If not, see <http://www.gnu.org/licenses/>.

## Contact

**Alex Goldhoorn**
Website: [alex.goldhoorn.net](https://alex.goldhoorn.net)
Email: alex@goldhoorn.net

_Note: The institutional email (agoldhoorn@iri.upc.edu) from the original research is no longer active._

## Credits

Developed at Institut de Robòtica i Informàtica Industrial (IRI), CSIC-UPC, Barcelona (2011-2016).
PhD supervised by Alberto Sanfeliu and René Alquézar.

Uses APPL 0.95 POMDP solver library (included in `trunk/ext_include/appl-0.95/`).
