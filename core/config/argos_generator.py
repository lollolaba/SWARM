#!/usr/bin/env python3
import random
import json
import math
import xml.etree.ElementTree as ET
from pathlib import Path

class ArgosGenerator:
    def __init__(
        self,
        template_file,
        output_file,
        num_robots,
        seed=1,
        duration=600,
        timestep=0.1,
        headless="false",
        channel_mode="REALISTIC",
        localization_algorithm="EKF",
        mac_protocol="ALOHA",
        arena_x=5.0,
        arena_y=5.0,
        min_distance=1.0,
        distribution="random",
        csv_file=""
    ):
        self.template_file = Path(template_file)
        self.output_file = Path(output_file)
        self.num_robots = num_robots
        self.seed = seed
        self.arena_x = arena_x
        self.arena_y = arena_y
        self.channel_mode = channel_mode
        self.localization_algorithm = localization_algorithm
        self.min_distance = min_distance
        self.distribution = distribution
        self.csv_file = csv_file
        self.seed=seed
        self.duration=duration
        self.timestep=timestep
        self.headless=headless
        self.mac_protocol = mac_protocol
        random.seed(seed)

    def generate_positions(self):
        if self.distribution == "line": return self.generate_line()
        if self.distribution == "circle": return self.generate_circle()
        if self.distribution == "csv": return self.generate_csv()
        
        positions = []
        attempts = 0

        while len(positions) < self.num_robots:
            if attempts > 10000:
                raise RuntimeError("Impossible to place robots with current constraints")
            x = random.uniform(-self.arena_x/2,self.arena_x/2)
            y = random.uniform(-self.arena_y/2,self.arena_y/2)
            valid = True

            for px,py in positions:
                dx = x-px
                dy = y-py
                dist = (dx*dx + dy*dy)**0.5
                if dist < self.min_distance:
                    valid=False
                    break
            if valid:positions.append((x,y))
            attempts += 1
        return positions

    def generate_line(self):
        positions=[]
        available_length = self.arena_x * 0.8
        if self.num_robots > 1: spacing = available_length/(self.num_robots-1)
        else: spacing = 0
        if spacing < self.min_distance: raise RuntimeError("Too many robots for line distribution with current arena size")
        start = -available_length/2
        for i in range(self.num_robots):
            x = start + i*spacing
            y = 0
            positions.append((x,y))
        return positions

    def generate_circle(self):
        positions=[]
        max_radius = min(self.arena_x,self.arena_y)/2 * 0.8
        required_radius = (self.num_robots *self.min_distance /(2*math.pi))
        radius = min(max_radius,required_radius)
        if radius < required_radius:
            raise RuntimeError("Too many robots for circle distribution")
        for i in range(self.num_robots):
            angle = (2 * math.pi * i / self.num_robots)
            x = radius*math.cos(angle)
            y = radius*math.sin(angle)
            positions.append((x,y))
        return positions

    def generate_csv(self):
        positions=[]
        with open(self.csv_file,"r") as f: 
            next(f)
            for line in f:
                _,x,y=line.strip().split(",")
                x=float(x)
                y=float(y)
                if abs(x)>self.arena_x/2 or abs(y)>self.arena_y/2:
                    raise RuntimeError(f"Robot outside arena: {x},{y}")
                positions.append((x,y))
        if len(positions)!=self.num_robots: raise RuntimeError("CSV robot number mismatch")
        return positions

    def insert_robots(self):
        self.output_file = self.get_incremental_filename()
        tree = ET.parse(self.template_file)
        root = tree.getroot()
        self.configure_experiment(root)
        self.configure_visualization(root)
        self.configure_loop_functions(root)
        arena = root.find("arena")
        if arena is None: raise RuntimeError("Arena node not found")
        arena.set("size",f"{self.arena_x},{self.arena_y},1")
        arena.set("center","0,0,0.5")
        positions = self.generate_positions()

        for old_robot in arena.findall("foot-bot"):arena.remove(old_robot)

        for i,(x,y) in enumerate(positions):
            robot = ET.SubElement(arena,"foot-bot",{"id":f"fb_{i}"})
            body = ET.SubElement(robot,"body",{"position":f"{x:.3f},{y:.3f},0"})
            controller = ET.SubElement(robot,"controller",{"config":"ctrl"})
        self.output_file.parent.mkdir(parents=True,exist_ok=True)
        tree.write(self.output_file,encoding="utf-8",xml_declaration=True)
        self.write_json(self.cfg,self.output_file)

    def configure_experiment(self, root):
        framework = root.find("framework")
        if framework is None:
            raise RuntimeError("Framework node not found")
        experiment = framework.find("experiment")
        if experiment is None:
            raise RuntimeError("Experiment node not found")
        ticks_per_second = int(1/self.timestep)
        experiment.set("length",str(int(self.duration)))
        experiment.set("ticks_per_second",str(ticks_per_second))
        experiment.set("random_seed",str(self.seed))

    def configure_visualization(self, root):
        visualization = root.find("visualization")
        if visualization is None : return
        if self.headless : root.remove(visualization)
        else: visualization.set("enabled","true")

    def configure_loop_functions(self, root):
        loop = root.find("loop_functions")
        if loop is None: raise RuntimeError("loop_functions node not found")
        cfg = self.cfg

        # Keep legacy loop-function attributes synchronized.
        loop.set("channel_mode",cfg["channel_mode"])
        loop.set("localization_mode",cfg["localization_algorithm"])
        loop.set("headless",str(cfg["headless"]).lower())
        loop.set("mac_protocol",cfg["mac_protocol"])
        experiment = loop.find("experiment_config")
        if experiment is None: experiment = ET.SubElement(loop,"experiment_config")

        def get_or_create(parent, tag):
            child = parent.find(tag)

            if child is None:
                child = ET.SubElement(
                    parent,
                    tag)

            return child
        simulation = get_or_create(experiment,"simulation")
        simulation.set("seed",str(cfg["seed"]))
        simulation.set("duration",str(cfg["duration"]))
        simulation.set("timestep",str(cfg["timestep"]))
        simulation.set("headless",str(cfg["headless"]).lower())
        robots = get_or_create(experiment,"robots")
        robots.set("number",str(cfg["num_robots"]))
        robots.set("distribution",cfg["distribution"])
        robots.set("min_distance",str(cfg["min_distance"]))
        robots.set("placement_seed",str(cfg["placement_seed"]))
        robots.set("csv",cfg["csv"])
        channel = get_or_create(experiment,"channel")
        channel.set("mode",cfg["channel_mode"])
        channel.set("range",str(cfg["channel_range"]))
        channel.set("noise",str(cfg["channel_noise"]))
        channel.set("attenuation",str(cfg["channel_attenuation"]))
        channel.set("speed_sound",str(cfg["speed_sound"]))
        channel.set("frequency_khz",str(cfg["frequency_khz"]))
        channel.set("bitrate_bps",str(cfg["bitrate_bps"]))
        channel.set("spreading_factor",str(cfg["spreading_factor"]))
        channel.set("source_level_db",str(cfg["source_level_db"]))
        channel.set("ambient_noise_db",str(cfg["ambient_noise_db"]))
        channel.set("snr_threshold_db",str(cfg["snr_threshold_db"]))
        channel.set("snr_transition_db",str(cfg["snr_transition_db"]))
        channel.set("fading_sigma_db",str(cfg["fading_sigma_db"]))
        channel.set("packet_overhead_bytes",str(cfg["packet_overhead_bytes"]))
        channel.set("neighbor_timeout",str(cfg["neighbor_timeout"]))
        channel.set("routing_timeout",str(cfg["routing_timeout"]))
        localization = get_or_create(experiment,"localization")
        localization.set("algorithm",cfg["localization_algorithm"])
        mac = get_or_create(experiment,"mac")
        mac.set("protocol",cfg["mac_protocol"])
        mac.set("aloha_probability",str(cfg["aloha_probability"]))
        mac.set("csma_backoff",str(cfg["csma_backoff"]))
        mac.set("csma_random_window",str(cfg["csma_random_window"]))
        mac.set("tdma_guard_interval",str(cfg["tdma_guard_interval"]))
        mac.set("tdma_payload_bytes",str(cfg["tdma_payload_bytes"]))
        mobility = get_or_create(experiment,"mobility")
        current = get_or_create(mobility,"current")
        current.set("strength",str(cfg["current_strength"]))
        current.set("factor",str(cfg["current_factor"]))
        drag = get_or_create(mobility,"drag")
        drag.set("x",str(cfg["drag_x"]))
        drag.set("y",str(cfg["drag_y"]))
        imu_noise = get_or_create(mobility,"imu_noise")
        imu_noise.set("sigma",str(cfg["imu_noise"]))

    def get_incremental_filename(self):
        path = self.output_file
        if not path.exists(): return path
        counter = 0
        while True:
            new_path = path.parent / (f"{path.stem}_{counter}{path.suffix}")
            if not new_path.exists() : return new_path
            counter += 1

    def write_json(self, cfg, index):json_file = self.output_file.with_suffix(".json")

        experiment = {
            "simulation": {
                "seed": cfg["seed"],
                "duration": cfg["duration"],
                "timestep": cfg["timestep"],
                "headless": cfg["headless"]
            },
            "robots": {
                "number": cfg["num_robots"],
                "distribution": cfg["distribution"],
                "min_distance": cfg["min_distance"],
                "placement_seed":
                    cfg["placement_seed"],
                "csv": cfg["csv"]
            },
            "arena": {
                "size_x": cfg["arena_x"],
                "size_y": cfg["arena_y"]
            },
            "channel": {
                "mode": cfg["channel_mode"],
                "range": cfg["channel_range"],
                "noise": cfg["channel_noise"],
                "attenuation": cfg["channel_attenuation"],
                "speed_sound": cfg["speed_sound"],
                "frequency_khz": cfg["frequency_khz"],
                "bitrate_bps": cfg["bitrate_bps"],
                "spreading_factor": cfg["spreading_factor"],
                "source_level_db": cfg["source_level_db"],
                "ambient_noise_db": cfg["ambient_noise_db"],
                "snr_threshold_db": cfg["snr_threshold_db"],
                "snr_transition_db": cfg["snr_transition_db"],
                "fading_sigma_db": cfg["fading_sigma_db"],
                "packet_overhead_bytes": cfg["packet_overhead_bytes"],
                "neighbor_timeout": cfg["neighbor_timeout"],
                "routing_timeout": cfg["routing_timeout"]
            },
            "localization": {
                "algorithm":
                    cfg["localization_algorithm"]
            },
            "mac": {
                "protocol": cfg["mac_protocol"],
                "aloha_probability":
                    cfg["aloha_probability"],
                "csma_backoff":
                    cfg["csma_backoff"],
                "csma_random_window":
                    cfg["csma_random_window"]
                "tdma_guard_interval":
                    cfg["tdma_guard_interval"],
                "tdma_payload_bytes":
                    cfg["tdma_payload_bytes"]
            },
            "mobility": {
                "current_strength":
                    cfg["current_strength"],
                "current_factor":
                    cfg["current_factor"],
                "drag_x": cfg["drag_x"],
                "drag_y": cfg["drag_y"],
                "imu_noise": cfg["imu_noise"]
            }
        }
        with open(json_file, "w") as file:json.dump(experiment,file,indent=4)

def load_experiment_config(filename):
    tree = ET.parse(filename)
    root = tree.getroot()

    simulation = root.find("simulation")
    robots = root.find("robots")
    arena = root.find("arena")
    channel = root.find("channel")
    localization = root.find("localization")
    mac = root.find("mac")
    mobility = root.find("mobility")

    required = {
        "simulation": simulation,
        "robots": robots,
        "arena": arena,
        "channel": channel,
        "localization": localization,
        "mac": mac
    }

    for name, node in required.items():
        if node is None:
            raise RuntimeError(
                f"Missing <{name}> in {filename}")

    config = {}

    config["seed"] = int(
        simulation.attrib.get(
            "seed",
            1))

    config["duration"] = float(
        simulation.attrib.get(
            "duration",
            600))

    config["timestep"] = float(
        simulation.attrib.get(
            "timestep",
            0.1))

    config["headless"] = (
        simulation.attrib
        .get("headless", "false")
        .lower() == "true"
    )

    config["num_robots"] = int(
        robots.attrib.get(
            "number",
            10))

    config["min_distance"] = float(
        robots.attrib.get(
            "min_distance",
            1.0))

    config["distribution"] = (
        robots.attrib.get(
            "distribution",
            "random")
    )

    config["placement_seed"] = int(
        robots.attrib.get(
            "placement_seed",
            config["seed"]))

    config["csv"] = robots.attrib.get(
        "csv",
        "")

    config["arena_x"] = float(
        arena.attrib.get(
            "size_x",
            5.0))

    config["arena_y"] = float(
        arena.attrib.get(
            "size_y",
            5.0))

    config["channel_mode"] = (
        channel.attrib
        .get("mode", "REALISTIC")
        .upper()
    )

    config["channel_range"] = float(
        channel.attrib.get(
            "range",
            50.0))

    config["channel_noise"] = float(
        channel.attrib.get(
            "noise",
            0.0))

    config["channel_attenuation"] = float(
        channel.attrib.get(
            "attenuation",
            0.05))

    config["speed_sound"] = float(
        channel.attrib.get(
            "speed_sound",
            1500.0))
    config["frequency_khz"] = float(
        channel.attrib.get(
            "frequency_khz",
            25.0))

    config["bitrate_bps"] = float(
        channel.attrib.get(
            "bitrate_bps",
            1000.0))

    config["spreading_factor"] = float(
        channel.attrib.get(
            "spreading_factor",
            1.5))

    config["source_level_db"] = float(
        channel.attrib.get(
            "source_level_db",
            150.0))

    config["ambient_noise_db"] = float(
        channel.attrib.get(
            "ambient_noise_db",
            90.0))

    config["snr_threshold_db"] = float(
        channel.attrib.get(
            "snr_threshold_db",
            10.0))

    config["snr_transition_db"] = float(
        channel.attrib.get(
            "snr_transition_db",
            2.0))

    config["fading_sigma_db"] = float(
        channel.attrib.get(
            "fading_sigma_db",
            1.5))

    config["packet_overhead_bytes"] = int(
        channel.attrib.get(
            "packet_overhead_bytes",
            32))

    config["neighbor_timeout"] = float(
        channel.attrib.get(
            "neighbor_timeout",
            2.0))

    config["routing_timeout"] = float(
        channel.attrib.get(
            "routing_timeout",
            3.0))

    config["localization_algorithm"] = (
        localization.attrib
        .get("algorithm", "NOISY")
        .upper()
    )

    config["mac_protocol"] = (
        mac.attrib
        .get("protocol", "CSMA")
        .upper()
    )

    config["aloha_probability"] = float(
        mac.attrib.get(
            "aloha_probability",
            0.7))

    config["csma_backoff"] = float(
        mac.attrib.get(
            "csma_backoff",
            0.2))

    config["csma_random_window"] = float(
        mac.attrib.get(
            "csma_random_window",
            0.2))
    config["tdma_guard_interval"] = float(
        mac.attrib.get(
            "tdma_guard_interval",
            0.0))

    config["tdma_payload_bytes"] = int(
        mac.attrib.get(
            "tdma_payload_bytes",
            0))

    current = (
        mobility.find("current")
        if mobility is not None
        else None
    )

    drag = (
        mobility.find("drag")
        if mobility is not None
        else None
    )

    imu_noise = (
        mobility.find("imu_noise")
        if mobility is not None
        else None
    )

    config["current_strength"] = float(
        current.attrib.get("strength", 0.0)
        if current is not None
        else 0.0)

    config["current_factor"] = float(
        current.attrib.get("factor", 0.1)
        if current is not None
        else 0.1)

    config["drag_x"] = float(
        drag.attrib.get("x", -0.05)
        if drag is not None
        else -0.05)

    config["drag_y"] = float(
        drag.attrib.get("y", -0.08)
        if drag is not None
        else -0.08)

    config["imu_noise"] = float(
        imu_noise.attrib.get("sigma", 0.02)
        if imu_noise is not None
        else 0.02)

    if config["duration"] <= 0.0:
        raise RuntimeError(
            "duration must be greater than zero")

    if config["timestep"] <= 0.0:
        raise RuntimeError(
            "timestep must be greater than zero")

    if config["num_robots"] <= 0:
        raise RuntimeError(
            "number of robots must be greater than zero")

    if config["channel_range"] <= 0.0:
        raise RuntimeError(
            "channel range must be greater than zero")

    if (
        config["channel_range"] <=
        config["min_distance"]
    ):
        print(
            "[CONFIG WARNING] Channel range is not "
            "greater than robot min_distance; "
            "neighbor discovery may return no links.")

    allowed_localization = {
        "IDEAL",
        "NOISY",
        "NONE",
        "EKF",
        "EKF_CI"
        "CONSENSUS",
        "PARTICLE_FILTER",
        "BAYESIAN",
        "FACTOR_GRAPH"
    }

    if (config["localization_algorithm"]not in allowed_localization): raise RuntimeError("Unsupported localization algorithm: "f"{config['localization_algorithm']}")

    allowed_channels = {
        "BASIC",
        "REALISTIC",
        "ADVANCED_SNR",
        "UNDERWATER_EXTREME",
        "THORP"
    }

    if config["channel_mode"] not in allowed_channels:
        raise RuntimeError(
            "Unsupported channel mode: "
            f"{config['channel_mode']}")


    if config["mac_protocol"] not in {"ALOHA","CSMA","TDMA"}:raise RuntimeError("Unsupported MAC protocol: "f"{config['mac_protocol']}")
    if config["tdma_guard_interval"] < 0.0: raise RuntimeError("tdma_guard_interval cannot be negative")
    if config["tdma_payload_bytes"] < 0: raise RuntimeError("tdma_payload_bytes cannot be negative")
    
    if config["frequency_khz"] <= 0.0: raise RuntimeError("frequency_khz must be greater than zero")

    if config["bitrate_bps"] <= 0.0:raise RuntimeError("bitrate_bps must be greater than zero")

    if not 1.0 <= config["spreading_factor"] <= 2.0: raise RuntimeError("spreading_factor must be between 1.0 and 2.0")

    if config["snr_transition_db"] <= 0.0: raise RuntimeError("snr_transition_db must be greater than zero")

    if config["fading_sigma_db"] < 0.0: raise RuntimeError("fading_sigma_db cannot be negative")
    return config

if __name__ == "__main__":
    cfg = load_experiment_config("../../experiments/experiment_config.xml")
    generator = ArgosGenerator(
        template_file="../../experiments/templates/swarm_template.argos",
        output_file="../../experiments/generated/swarm_generated.argos",
        num_robots=cfg["num_robots"],
        seed=cfg["seed"],
        arena_x=cfg["arena_x"],
        arena_y=cfg["arena_y"],
        min_distance=cfg["min_distance"],
        distribution=cfg["distribution"],
        csv_file=cfg["csv"],
        duration=cfg["duration"],
        timestep=cfg["timestep"],
        headless=cfg["headless"],
        channel_mode=cfg["channel_mode"],
        localization_algorithm=cfg["localization_algorithm"],
        mac_protocol=cfg["mac_protocol"]
    )
    generator.cfg = cfg
    generator.insert_robots()
    print("Experiment generated")