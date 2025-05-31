#include <iostream>
#include <vector>      // Used for dynamic list of sources and loads
#include <map>         // For mapping component names to breakers
#include <set>         // For managing fault names
#include <string>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace SmartGrid {  //  Namespace usage

// -------------------------
// Breaker Class
// -------------------------
class Breaker {
    std::string id;
    bool tripped;
public:
    Breaker() : id("unknown"), tripped(false) {}
    Breaker(const std::string& i) : id(i), tripped(false) {}
    void trip() { tripped = true; }
    void reset() { tripped = false; }
    bool isTripped() const { return tripped; }
    std::string getId() const { return id; }
};

// -------------------------
// Abstract Base Class: PowerComponent
// -------------------------
class PowerComponent {
protected:
    std::string name;
    bool status;
public:
    PowerComponent(const std::string& n) : name(n), status(true) {}
    virtual ~PowerComponent() {}  // Virtual destructor
    virtual void simulate() = 0;  // Pure virtual function (abstract class)
    std::string getName() const { return name; }
    bool isConnected() const { return status; }
    void disconnect() { status = false; }
    void reconnect() { status = true; }
};

// -------------------------
// Concrete Derived Class: PowerSource
// -------------------------
class PowerSource : public PowerComponent {
protected:
    float powerOutput;
    bool renewable;
public:
    PowerSource(const std::string& n, float p, bool r) : PowerComponent(n), powerOutput(p), renewable(r) {}
    void simulate() override {  // Overridden method for polymorphism
        if (status)
            std::cout << "[Source] " << name << " generating " << powerOutput << "kW\n";
    }
    float getPowerOutput() const { return powerOutput; }
};

// -------------------------
// Further Derived Class: SolarSource
// -------------------------
class SolarSource : public PowerSource {
public:
    SolarSource(const std::string& n) : PowerSource(n, 50.0f, true) {}
    void simulate() override {
        if (status) {
            powerOutput = 20 + std::rand() % 30;  // Fluctuating behavior
            std::cout << "[Solar] " << name << " output: " << powerOutput << "kW\n";
        }
    }
};

// -------------------------
// Load Class (not derived from base, supports simulation and disconnection)
// -------------------------
class Load {
    std::string name;
    float demand;
    bool connected;
    int priority;
public:
    Load(const std::string& n, float d, int p = 5) : name(n), demand(d), connected(true), priority(p) {}
    std::string getName() const { return name; }
    float getRawDemand() const { return demand; }
    bool isConnected() const { return connected; }
    int getPriority() const { return priority; }
    void disconnect() { connected = false; }
    void reconnect() { connected = true; }
    void simulate() const {
        std::cout << "[Load] " << name << ": " << demand << "kW, Priority: " << priority
                  << ", Connected: " << (connected ? "Yes" : "No") << "\n";
    }
};

// -------------------------
// GridManager Class: Core controller
// -------------------------
class GridManager {
    std::vector<PowerComponent*> sources;  // Polymorphism via base class pointers
    std::vector<Load> loads;
    std::map<std::string, Breaker> breakers;
    std::set<std::string> faults;
public:
    ~GridManager() {
        for (auto src : sources)
            delete src;  //  Memory management (requirement 5)
    }

    void addSource(PowerComponent* src) {
        sources.push_back(src);
        breakers.emplace(src->getName(), Breaker(src->getName()));
        simulate();
    }

    void addLoad(const Load& l) {
        loads.push_back(l);
        breakers.emplace(l.getName(), Breaker(l.getName()));
    }

    //  Simulation logic using polymorphism
    void simulate() {
        std::cout << "\n=== Cycle ===\n[Log] Simulation Start\n";
        float totalPower = 0, totalDemand = 0;

        for (auto src : sources) {
            if (!breakers[src->getName()].isTripped()) {
                src->simulate();
                PowerSource* ps = dynamic_cast<PowerSource*>(src);
                if (ps && src->isConnected()) totalPower += ps->getPowerOutput();
            }
        }

        for (const auto& l : loads) {
            if (!breakers[l.getName()].isTripped()) {
                l.simulate();
                if (l.isConnected()) totalDemand += l.getRawDemand();
            }
        }

        std::cout << "[Log] Total Power: " << totalPower << "kW\n";
        std::cout << "[Log] Total Demand: " << totalDemand << "kW\n";

        // Load shedding logic
        if (totalPower < totalDemand) {
            std::cout << "[Warning] Power Deficit Detected. Tripping loads based on priority.\n";
            std::vector<Load*> sortedLoads;
            for (auto& l : loads)
                if (l.isConnected()) sortedLoads.push_back(&l);
            std::sort(sortedLoads.begin(), sortedLoads.end(), [](Load* a, Load* b) {
                return a->getPriority() > b->getPriority();
            });

            for (auto* l : sortedLoads) {
                l->disconnect();
                breakers[l->getName()].trip();
                std::cout << "[Trip] Load " << l->getName() << " tripped due to overload.\n";
                totalDemand -= l->getRawDemand();
                if (totalPower >= totalDemand) break;
            }
        } else {
            // Reconnect loads in priority order
            std::vector<Load*> disconnectedLoads;
            for (auto& l : loads) {
                if (!l.isConnected() && !breakers[l.getName()].isTripped())
                    disconnectedLoads.push_back(&l);
            }
            std::sort(disconnectedLoads.begin(), disconnectedLoads.end(), [](Load* a, Load* b) {
                return a->getPriority() < b->getPriority();
            });

            for (auto* l : disconnectedLoads) {
                if (totalPower >= totalDemand + l->getRawDemand()) {
                    l->reconnect();
                    std::cout << "[Reconnect] Load " << l->getName() << " reconnected.\n";
                    totalDemand += l->getRawDemand();
                }
            }
        }

        for (const auto& f : faults)
            std::cout << "[Log] Active Fault: " << f << "\n";

        std::cout << "[Log] Simulation End\n";
    }

    // -------------------
    // Manual Fault Controls
    // -------------------
    void injectManualFault() {
        std::cout << "Select target to fault:\n";
        for (size_t i = 0; i < loads.size(); ++i)
            std::cout << "L" << i << ": Load: " << loads[i].getName() << "\n";
        for (size_t i = 0; i < sources.size(); ++i)
            std::cout << "S" << i << ": Source: " << sources[i]->getName() << "\n";
        std::string input;
        std::cin >> input;
        std::string name;
        if (input[0] == 'L') name = loads[std::stoi(input.substr(1))].getName();
        else if (input[0] == 'S') name = sources[std::stoi(input.substr(1))]->getName();
        faults.insert(name);
        breakers[name].trip();
        std::cout << "[Fault] Injected at " << name << "\n";
    }

    void resolveManualFault() {
        std::cout << "Active faults:\n";
        int i = 0;
        for (const auto& f : faults)
            std::cout << i++ << ": " << f << "\n";
        int index;
        std::cin >> index;
        auto it = faults.begin();
        std::advance(it, index);
        breakers[*it].reset();
        faults.erase(it);
        std::cout << "[Fault] Resolved: " << *it << "\n";
        simulate();
    }

    void disconnectLoad(size_t index) { loads[index].disconnect(); }
    void reconnectLoad(size_t index) { loads[index].reconnect(); }
    void showBreakers() const {
        std::cout << "\n[Breaker Status]\n";
        for (const auto& [k, b] : breakers)
            std::cout << k << ": " << (b.isTripped() ? "TRIPPED" : "OK") << "\n";
    }
    const std::vector<Load>& getLoads() const { return loads; }
};

// -------------------------
// Operator Overloading
// -------------------------
std::ostream& operator<<(std::ostream& os, const Load& load) {
    os << "[Load] " << load.getName() << ": " << load.getRawDemand() << "kW, Priority: "
       << load.getPriority() << ", Connected: " << (load.isConnected() ? "Yes" : "No");
    return os;
}

std::ostream& operator<<(std::ostream& os, const PowerSource& source) {
    os << "[Source] " << source.getName() << ": " << source.getPowerOutput() << "kW";
    return os;
}

} // namespace SmartGrid

// -------------------------
// Main Application Entry
// -------------------------
int main() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    using namespace SmartGrid;

    GridManager gm;
    gm.addSource(new SolarSource("SolarFarm-A"));
    gm.addSource(new PowerSource("HydroStation", 60.0f, false));
    gm.addLoad(Load("Factory-A", 30, 2));
    gm.addLoad(Load("House-B", 15, 1));
    gm.addLoad(Load("Shop-C", 10, 3));

    int choice;
    do {
        std::cout << "\n=== Smart Grid Menu ===\n";
        std::cout << "1. Run simulation cycle\n2. Inject fault\n3. Resolve fault\n";
        std::cout << "4. Disconnect load\n5. Reconnect load\n6. Show breaker states\n";
        std::cout << "7. Add new load\n8. Add new source\n0. Exit\nEnter choice: ";
        std::cin >> choice;

        if (choice == 1) gm.simulate();
        else if (choice == 2) gm.injectManualFault();
        else if (choice == 3) gm.resolveManualFault();
        else if (choice == 4) {
            const auto& loads = gm.getLoads();
            for (size_t i = 0; i < loads.size(); ++i)
                std::cout << i << ": " << loads[i].getName() << std::endl;
            size_t index;
            std::cin >> index;
            gm.disconnectLoad(index);
        }
        else if (choice == 5) {
            const auto& loads = gm.getLoads();
            for (size_t i = 0; i < loads.size(); ++i)
                std::cout << i << ": " << loads[i].getName() << std::endl;
            size_t index;
            std::cin >> index;
            gm.reconnectLoad(index);
        }
        else if (choice == 6) gm.showBreakers();
        else if (choice == 7) {
            std::string name;
            float demand;
            int priority;
            std::cin >> name >> demand >> priority;
            gm.addLoad(Load(name, demand, priority));
        }
        else if (choice == 8) {
            std::string name;
            float power;
            int type;
            std::cin >> name >> power >> type;
            if (type == 1)
                gm.addSource(new SolarSource(name));
            else
                gm.addSource(new PowerSource(name, power, (type == 2 || type == 3)));
        }
        else if (choice == 0) std::cout << "Exiting simulation.\n";
        else std::cout << "Invalid choice.\n";
    } while (choice != 0);

    return 0;
}
