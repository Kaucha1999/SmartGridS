# Smart Grid Simulator

A C++ smart grid power management simulator demonstrating object-oriented programming concepts including inheritance, polymorphism, and automatic load balancing.

## Features

- **Power Sources**: Solar panels with variable output, hydroelectric stations
- **Load Management**: Automatic load shedding based on priority levels
- **Fault Simulation**: Inject and resolve component faults
- **Circuit Breakers**: Trip/reset protection for all components
- **Real-time Balancing**: Matches power generation to demand

## Quick Start

```bash
g++ -o sgs Smartgridsimulator.cpp
./sgs
```

## Menu Options

1. **Run Simulation** - Execute one power balancing cycle
2. **Inject Fault** - Simulate component failure
3. **Resolve Fault** - Fix active faults
4. **Disconnect Load** - Manually disconnect power consumers
5. **Reconnect Load** - Restore disconnected loads
6. **Show Breakers** - Display all circuit breaker states
7. **Add Load** - Create new power consumer
8. **Add Source** - Create new power generator

## How It Works

- System automatically balances power supply vs demand
- When demand exceeds supply, loads are shed by priority (higher numbers disconnected first)
- When surplus power is available, loads reconnect by priority (lower numbers first)
- Solar sources have random output variation (20-50kW)
- All components have circuit breaker protection

## Default Setup

- **Sources**: Solar Farm (variable), Hydro Station (60kW)
- **Loads**: Factory (30kW, priority 2), House (15kW, priority 1), Shop (10kW, priority 3)

The simulator runs continuous cycles, showing real-time power balance and component status.
