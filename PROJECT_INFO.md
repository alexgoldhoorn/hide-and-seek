# Hide-and-Seek Robot Search Simulator - Website Integration Info

This document contains all information needed to add this project to alex.goldhoorn.net

## Project Overview

**Title:** Hide-and-Seek Robot Search Simulator

**Short Description:** Simulation framework for probabilistic robot search and tracking with multiple solver implementations (POMCP, MOMDP, Particle Filters, Kalman Filters, and heuristics)

**Type:** Research Code / Open Source Software

**Status:** Published (Historical research code from 2011-2016 PhD)

**GitHub Repository:** https://github.com/alexgoldhoorn/hide-and-seek

**License:** LGPL v3.0

## Website Integration

### Suggested Location
- **URL Path:** `/projects/hide-and-seek/`
- **Main File:** Use `website-project-page.html` as `index.html`

### Links to Update

1. **Projects Index** (`/projects/index.html`)
   - Add card for "Hide-and-Seek Robot Search Simulator"
   - Category: Research Tools / Robotics
   - Link to `/projects/hide-and-seek/`

2. **Thesis Page** (`/thesis/index.html`)
   - Add link to GitHub repository in "Related Materials" section
   - Text: "Source Code on GitHub" or similar

3. **Publications Pages**
   - Consider adding GitHub link to relevant publication pages:
     - `/publications/ar2016/`
     - `/publications/ras2016/`
     - `/publications/find-and-follow/`
     - `/publications/robot2013/`

## Key Features to Highlight

- **Multiple Solvers:**
  - POMCP (online Monte-Carlo planning)
  - MOMDP (mixed-observability planning)
  - Particle Filter tracking
  - Kalman Filter tracking
  - Multi-robot cooperative search
  - Heuristic baselines

- **Infrastructure:**
  - Client-server multiplayer architecture
  - Qt graphical client for human players
  - MySQL logging for experiment analysis
  - ROS integration for real robots

- **Research Data:**
  - Maps from published experiments
  - Pre-computed policies
  - Test scenarios and benchmarks

## Related Resources

**Interactive Demo (already exists):**
- URL: `/projects/hide-and-seek-belief/`
- Description: Web-based visualization of belief tracking algorithms
- Relationship: Companion demo for the research code

**PhD Thesis:**
- URL: `/thesis/`
- PDF: `/thesis/Goldhoorn2017_PhD_thesis.pdf`
- Title: "Searching and Tracking of Humans in Urban Environments by Humanoid Robots"

**Publications:**
- Autonomous Robots 2017: `/publications/ar2016/`
- Robotics and Autonomous Systems 2017: `/publications/ras2016/`
- Humanoids 2014: `/publications/find-and-follow/`
- ROBOT 2013: `/publications/robot2013/`

## SEO & Metadata

**Keywords:**
- POMCP
- POMDP
- particle filters
- robot search
- human tracking
- hide-and-seek
- probabilistic robotics
- MOMDP
- Kalman filter
- multi-robot coordination

**Meta Description:**
"C++ implementation of POMCP, MOMDP, Particle Filters, Kalman Filters and heuristic algorithms for robot search and tracking developed during PhD research (Goldhoorn 2017)"

## Important Notes

1. **Historical Code Notice:** Emphasize this is research code from 2011-2016 with outdated dependencies
2. **Contact Info:** Use alex@goldhoorn.net (IRI email agoldhoorn@iri.upc.edu is no longer active)
3. **Dual Resources:** Make clear distinction between:
   - This project: Research code repository (GitHub)
   - `/projects/hide-and-seek-belief/`: Interactive web demo

## Suggested Project Card Text

**For Projects Index:**

```
Hide-and-Seek Robot Search Simulator
C++ simulation framework with POMCP, MOMDP, Particle Filters, and Kalman Filter implementations for probabilistic robot search and tracking. Complete PhD research code (2011-2016).
[View Code] [Try Demo]
```

## Citation

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

## File Checklist

Files provided for website integration:
- ✅ `website-project-page.html` - Main project page (use as index.html)
- ✅ `README.md` - GitHub README (reference only)
- ✅ `LICENSE` - LGPL v3 license file (reference only)
- ✅ `PROJECT_INFO.md` - This file (integration guide)

## Cross-Linking Strategy

**From Other Pages → Hide-and-Seek Code:**
- Thesis page: Link to GitHub repo
- Publications: Link to implementation
- Hide-and-seek-belief demo: Link to source code

**From Hide-and-Seek Code → Other Pages:**
- Link to interactive demo
- Link to thesis
- Link to publications
- Link back to projects index

## Visual Suggestions

**Icons to use:**
- 🤖 Robot/AI
- 🔍 Search
- 🎯 Target/tracking
- 🗺️ Map/environment
- 📊 Data/analysis

**Color scheme:** Use website's standard blue theme (matches robotics/CS topics)
