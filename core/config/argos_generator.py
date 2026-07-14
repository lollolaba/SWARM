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
        if loop is None:
            raise RuntimeError("loop_functions node not found")
        loop.set("channel_mode",self.channel_mode)
        loop.set("localization_mode",self.localization_algorithm)
        loop.set("headless",str(self.headless).lower())
        loop.set("mac_protocol",self.mac_protocol)
    def get_incremental_filename(self):
        path = self.output_file
        if not path.exists(): return path
        counter = 0
        while True:
            new_path = path.parent / (f"{path.stem}_{counter}{path.suffix}")
            if not new_path.exists() : return new_path
            counter += 1

    def write_json(self, cfg, index):
        json_file = self.output_file.with_suffix(".json")
        experiment = {
            "simulation":{
                "seed": cfg["seed"],
                "duration": cfg.get("duration",600),
                "headless": cfg.get("headless",False)
            },
            "robots":{
                "number": cfg["num_robots"],
                "distribution": cfg["distribution"],
                "min_distance": cfg["min_distance"],
                "placement_seed": cfg.get("placement_seed",cfg["seed"])
            },
            "arena":{
                "size_x": cfg["arena_x"],
                "size_y": cfg["arena_y"]
            },
            "channel":{
                "mode": cfg.get("channel_mode","REALISTIC"),
                "range": cfg["channel_range"]
            },
            "localization":{
                "algorithm":cfg.get("localization","EKF")
            },
            "mac":{
                "protocol":cfg.get("mac","ALOHA")
            }
        }
        with open(json_file,"w") as f: json.dump(experiment,f,indent=4)

def load_experiment_config(filename):
    tree = ET.parse(filename)
    root = tree.getroot()
    simulation = root.find("simulation")
    robots = root.find("robots")
    arena = root.find("arena")
    channel = root.find("channel")
    localization = root.find("localization")

    config = {}
    config["seed"] = int(simulation.attrib.get("seed",1))
    config["num_robots"] = int(robots.attrib.get("number",10))
    config["min_distance"] = float(robots.attrib.get("min_distance",1.0))
    config["arena_x"] = float(arena.attrib.get("size_x",5))
    config["arena_y"] = float(arena.attrib.get("size_y",5))
    config["channel_range"]= float(channel.attrib.get("range",50))
    config["distribution"] = robots.attrib.get("distribution","random")
    config["placement_seed"] = int(robots.attrib.get("placement_seed",config["seed"]))
    config["csv"] = robots.attrib.get("csv","")
    config["duration"] = float(simulation.attrib.get("duration",600))
    config["timestep"] = float(simulation.attrib.get("timestep",0.1))
    config["headless"] = simulation.attrib.get("headless","false").lower() == "true"
    config["seed"] = int(simulation.attrib.get("seed",1))
    config["channel_mode"] = channel.attrib.get("mode","REALISTIC")
    config["localization_algorithm"] = localization.attrib.get("algorithm","EKF")
    config["mac_protocol"] = root.find("mac").attrib.get("protocol","ALOHA")
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