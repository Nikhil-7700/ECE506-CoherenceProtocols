# Cache Coherence Protocol Simulator

This repository contains a cache coherence simulator for analyzing and comparing the **Modified MSI** and **Dragon** protocols. The simulator models a functional 4-processor cache system to observe coherence behaviors using trace-driven simulation.

## Project Summary

The simulator evaluates performance metrics such as cache hits/misses, memory transactions, and coherence actions under two protocols:

- **Modified MSI Protocol** (M, S, E states with invalidation)
- **Dragon Protocol** (M, E, Shared-Modified, Shared-Clean without invalidation)

> **Goal:** Understand and compare how coherence protocols influence memory system behavior in a parallel processor environment.

- **Course**: ECE 506/406 – Architecture of Parallel Computers
- **Author**: Padmanabha Nikhil Bhimavarapu  
- **Unity ID**: `pbhimav`

## Simulator Usage

### Compile

```bash
cd src/
make
