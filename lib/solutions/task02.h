#pragma once

#include <cstdint>
#include <optional>
#include <set>
#include <vector>
#include <unordered_map>
#include <queue>
#include <algorithm>
#include <limits>

#include "Support.h"

using NumberGraph = ex6::task2::NumberGraph;
using WordEditGraph = ex6::task2::WordEditGraph;
using Spelling = ex6::task2::Spelling;

// Type trait for graph operations
// This provides a generic interface for different graph types
template<typename Graph>
struct GraphTraits;

// Specialization for NumberGraph
template<>
struct GraphTraits<NumberGraph> {
    using Node = NumberGraph::NodeID;

    // Get all neighbors of a node with their edge weights
    static std::vector<std::pair<Node, size_t>> getNeighbors(
        const NumberGraph& graph,
        const Node& node) {
        const auto& edges = graph.getOutgoing(node);
        std::vector<std::pair<Node, size_t>> neighbors;
        neighbors.reserve(edges.size());
        for (const auto& edge : edges) {
            neighbors.push_back({edge.target, edge.weight});
        }
        return neighbors;
    }
};

// Specialization for WordEditGraph (fully connected graph)
template<>
struct GraphTraits<WordEditGraph> {
    using Node = size_t;  // Nodes are indices into the vector

    // Get all neighbors (all other nodes in fully connected graph)
    static std::vector<std::pair<Node, size_t>> getNeighbors(
        const WordEditGraph& graph,
        const Node& node) {
        std::vector<std::pair<Node, size_t>> neighbors;
        neighbors.reserve(graph.size() - 1);

        for (size_t i = 0; i < graph.size(); ++i) {
            if (i != node) {
                // Weight is square of letter frequency similarity
                size_t sim = ex6::task2::similarity(graph[node], graph[i]);
                size_t weight = sim * sim;
                neighbors.push_back({i, weight});
            }
        }
        return neighbors;
    }
};

// Dijkstra's shortest path algorithm using type traits
template<typename Graph>
std::vector<typename GraphTraits<Graph>::Node>
shortestPath(const Graph& graph,
             const typename GraphTraits<Graph>::Node& source,
             const typename GraphTraits<Graph>::Node& target) {
    using Node = typename GraphTraits<Graph>::Node;
    using Traits = GraphTraits<Graph>;

    // Source equals target: return single-node path
    if (source == target) {
        return {source};
    }

    // Distance from source to each node
    std::unordered_map<Node, size_t> distances;
    distances[source] = 0;

    // Previous node in shortest path (for reconstruction)
    std::unordered_map<Node, Node> previous;

    // Priority queue: (distance, node) pairs, ordered by distance
    using PQElement = std::pair<size_t, Node>;
    std::priority_queue<PQElement,
                        std::vector<PQElement>,
                        std::greater<PQElement>> pq;
    pq.push({0, source});

    // Track visited nodes
    std::set<Node> visited;

    // Dijkstra's algorithm
    while (!pq.empty()) {
        auto [currentDist, currentNode] = pq.top();
        pq.pop();

        // Skip if already visited
        if (visited.count(currentNode)) {
            continue;
        }
        visited.insert(currentNode);

        // Found target: reconstruct path
        if (currentNode == target) {
            std::vector<Node> path;
            Node current = target;

            // Trace back from target to source
            while (true) {
                path.push_back(current);
                if (current == source) {
                    break;
                }
                current = previous[current];
            }

            // Reverse to get source-to-target order
            std::reverse(path.begin(), path.end());

            // Return empty if path has 128+ nodes
            if (path.size() >= 128) {
                return {};
            }

            return path;
        }

        // Explore neighbors
        auto neighbors = Traits::getNeighbors(graph, currentNode);

        for (const auto& [neighbor, weight] : neighbors) {
            if (visited.count(neighbor)) {
                continue;
            }

            size_t newDist = currentDist + weight;

            // Update if found shorter path
            if (!distances.count(neighbor) || newDist < distances[neighbor]) {
                distances[neighbor] = newDist;
                previous[neighbor] = currentNode;
                pq.push({newDist, neighbor});
            }
        }
    }

    // No path found
    return {};
}
