# Hide-and-Seek Simulator

A comprehensive simulation framework for probabilistic search and tracking of humans in urban environments using POMDPs, POMCP, and particle filters. This code was developed as part of a PhD thesis on robot-human hide-and-seek games.

[![License: LGPL v3](https://img.shields.io/badge/License-LGPL%20v3-blue.svg)](https://www.gnu.org/licenses/lgpl-3.0)
[![Language: C++11](https://img.shields.io/badge/C++-11-00599C.svg)](https://isocpp.org/)
[![Platform: Linux](https://img.shields.io/badge/Platform-Linux-lightgrey.svg)](https://www.linux.org/)

## Overview

This project contains a complete hide-and-seek game simulator and seeker algorithms (POMDP "solvers") for robot search and tracking tasks. The simulator includes:

- **Server-client architecture** for multiplayer hide-and-seek games
- **Graphical client** for human players to play against robot seekers
- **Multiple solver implementations**:
  - MOMDP (Mixed Observability MDP) with offline planning
  - POMCP (Partially Observable Monte-Carlo Planning) for online real-time planning
  - Particle Filters and Kalman Filters for tracking
  - Heuristic seekers (smart seeker, follower, etc.)
  - Multi-robot cooperative search
- **Extensive logging** to MySQL database for analysis
- **ROS integration** for real robot experiments

The code can be run as a standalone console simulator or integrated with ROS for real robot deployments.

## Related Publications

This code was developed for the following PhD thesis and publications:

**PhD Thesis:**
> Alex Goldhoorn. *"Searching and Tracking of Humans in Urban Environments by Humanoid Robots"*.
> Institut de Robòtica i Informàtica Industrial, CSIC-UPC, Barcelona, Spain, 2017.
> [PDF](https://alex.goldhoorn.net/thesis/Goldhoorn2017_PhD_thesis.pdf) | [Website](https://alex.goldhoorn.net/thesis/)

**Key Publications:**
- **Autonomous Robots 2017** - [Searching and Tracking People with Cooperative Mobile Robots](https://alex.goldhoorn.net/publications/ar2016/)
- **Robotics and Autonomous Systems 2017** - [Searching and Tracking People in Urban Environments with Static and Dynamic Obstacles](https://alex.goldhoorn.net/publications/ras2016/)
- **Humanoids 2014** - [Continuous Real Time POMCP to Find-and-Follow People](https://alex.goldhoorn.net/publications/find-and-follow/)
- **ROBOT 2013** - [Analysis of Methods for Playing Human Robot Hide-and-Seek](https://alex.goldhoorn.net/publications/robot2013/)

**Interactive Demo:**
Try the [Hide-and-Seek Belief Simulation](https://alex.goldhoorn.net/projects/hide-and-seek-belief/) to compare POMCP, Particle Filters, and frontier exploration algorithms.

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

## Project Structure

```
hide-and-seek/
├── trunk/              # Main development branch
│   ├── src/           # Source code (~474 C++ files)
│   ├── include/       # Headers (includes APPL 0.95 POMDP solver)
│   ├── sql/           # Database schemas for game logging
│   ├── data/          # Maps, policies, and test data
│   ├── scripts/       # Build and utility scripts
│   └── *.pro          # Qt project files
├── branches/          # Development branches (MOMDP variants)
├── tags/              # Release tags (CR-POMCP versions)
└── data/              # Shared data (maps, policies, test files)
```

## Requirements

### Original Requirements (2013-2016)

This code was developed between 2011-2016 and has the following **historical dependencies**:

- **g++ 4.8** or newer
- **Qt 5** (for GUI and server)
- **MySQL 5.6+** (for game logging)
- **OpenCV 2.3** (< 3.0, though OpenCV 3 is supported with `CONFIG+=USE_OPENCV3`)
- **Eigen v3**
- **iriutils** (IRI robotics library - see below)

> **⚠️ Note on Outdated Dependencies:**
> These requirements are ~10 years old. For modern systems (2024+), you will likely need to:
> - Use g++ 11+ with `-std=c++11` flag
> - Update to OpenCV 4.x (may require API changes)
> - Use MySQL 8.x (mostly backwards compatible)
> - Qt 5.15+ or Qt 6 (may require minor updates)

### External Dependency: iriutils

The code depends on `iriutils`, an IRI robotics utility library:

```bash
# Original SVN repository (may require IRI access):
svn co https://devel.iri.upc.edu/labrobotica/drivers/iriutils/trunk

# Build iriutils:
cd iriutils/trunk
mkdir build && cd build
cmake ..
sudo make install
```

> **Note:** The iriutils SVN repository may not be publicly accessible. If you encounter access issues, please contact the repository maintainers or check for alternative installations.

## Building

### Library Version (CMake)

To build just the library for use with ROS or other applications:

```bash
cd trunk/build
cmake ..
make
```

### Console Simulator (Qt)

The simulator uses Qt project files. Build with:

**Using Qt Creator:**
1. Open desired `.pro` file in Qt Creator
2. Configure for Release or Debug
3. Build

**Using qmake:**
```bash
cd trunk
qmake hsmomdp.pro      # Automated seeker
qmake hsserver.pro     # Game server
qmake hsclient.pro     # GUI client
qmake hsautohider.pro  # Automated hider
make
```

**Build all at once:**
```bash
cd trunk
./scripts/qmake-all.sh
```

### Qt Project Files

- `hsmomdp.pro` - Console automated seeker with POMCP/MOMDP solvers
- `hsserver.pro` - Game server for multiplayer games
- `hsclient.pro` - Graphical client for human players
- `hsautohider.pro` - Automated hider agent
- `actiongen.pro` - Random action generator for testing

## Database Setup (Optional)

For game logging and analysis:

1. Install MySQL server:
```bash
sudo apt-get install mysql-server mysql-client
```

2. Create the database:
```bash
mysql -u root -p < trunk/sql/hsgamelog-db-new-multi.sql
```

> **Security Note:** The SQL setup script creates a user with password `hsserver_us3r_p@ss`. This is fine for local development/research, but change it for any public deployment.

## Quick Start Example

```bash
# 1. Start the server
cd trunk
./bin/hsserver

# 2. In another terminal, start an automated seeker with POMCP:
./bin/hsmomdp -Gh -scd -A 1 -u TestUser

# Command breakdown:
# -Gh        : Game mode = hide-and-seek
# -scd       : Solver = POMCP with discrete states
# -A 1       : Use map ID 1
# -u TestUser: Username for logging
```

For detailed command-line options, run:
```bash
./bin/hsmomdp -h
```

## Solver Types

The simulator supports multiple solving approaches:

| Solver | Flag | Description |
|--------|------|-------------|
| **POMCP** | `-scd` | Online POMCP with discrete states |
| **POMCP Continuous** | `-scc` | POMCP with continuous state space |
| **MOMDP** | `-sl` | Layered MOMDP with offline planning |
| **Particle Filter** | `-sPu` | Particle filter tracker |
| **Kalman Filter** | `-sKr` | Linear Kalman filter |
| **Smart Seeker** | `-ss` | Heuristic-based seeker |
| **Multi-robot** | `-sMc` | Multi-robot cooperative search |

## ROS Integration

The code includes ROS integration for real robot experiments. See `trunk/README.txt` lines 149-177 for ROS simulation instructions using the Tibi-Dabo robot platform.

## Interactive Visualization

For a web-based visualization of the belief tracking algorithms, visit:
**[Hide-and-Seek Belief Simulation](https://alex.goldhoorn.net/projects/hide-and-seek-belief/)**

This interactive demo lets you compare:
- POMCP (Monte-Carlo planning)
- Particle Filter tracking
- Frontier exploration
- Smart heuristic seekers

## Documentation

Detailed documentation is available in:
- `trunk/README.txt` - Complete command-line reference (17KB)
- `trunk/doc/` - Doxygen configuration files
- [PhD Thesis](https://alex.goldhoorn.net/thesis/) - Theoretical background and analysis
- [Thesis Videos](https://alex.goldhoorn.net/thesis/hide-and-seek-pomcp/) - Experimental demonstrations

## License

Copyright (C) 2011-2016 Institut de Robòtica i Informàtica Industrial, CSIC-UPC.

This software is free software: you can redistribute it and/or modify it under the terms of the **GNU Lesser General Public License** as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this program. If not, see <http://www.gnu.org/licenses/>.

## Author & Contact

**Alex Goldhoorn**
Institut de Robòtica i Informàtica Industrial, CSIC-UPC
Barcelona, Spain

- Website: [alex.goldhoorn.net](https://alex.goldhoorn.net)
- Original Email: agoldhoorn@iri.upc.edu, alex@goldhoorn.net

For questions about the code or research, please refer to the [publications](https://alex.goldhoorn.net/publications/) or [thesis](https://alex.goldhoorn.net/thesis/) pages.

## Acknowledgments

This work was developed at the Institut de Robòtica i Informàtica Industrial (IRI), CSIC-UPC, Barcelona, as part of a PhD thesis supervised by Alberto Sanfeliu and René Alquézar.

The POMDP solver uses the APPL 0.95 library, included in `trunk/ext_include/appl-0.95/`.

## Historical Note

This code represents research from 2011-2016 and uses dependencies from that era. While the algorithms and approaches remain valid and scientifically relevant, the software dependencies are outdated by modern standards (2024+). Consider this when building and deploying.

For a modern, interactive demonstration of the core algorithms, see the [web-based belief simulation](https://alex.goldhoorn.net/projects/hide-and-seek-belief/).
