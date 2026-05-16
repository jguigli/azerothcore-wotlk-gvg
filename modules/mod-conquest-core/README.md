# GvG Core Module

A Guild vs Guild (GvG) module for AzerothCore that adds guild relationship tracking and statistics.

## Features

- Track guild relationships (hostile, friendly, neutral)
- Track player kill/death statistics in guild battles
- Commands to manage guild relations

## Commands

- `.gvg reload` - Reload guild relations from database
- `.gvg relation set <guildA> <guildB> <relation>` - Set relation between guilds (-1=hostile, 0=neutral, 1=friendly)
- `.gvg relation list` - List guild relations

## Database Tables

The module creates 4 tables:
- `gvg_player_stats` - Player kill/death statistics
- `gvg_guild_stats` - Guild statistics
- `gvg_guild_relations` - Guild relationship data
- `gvg_global_config` - Module configuration

## Configuration

See `gvg_core.conf.dist` for configuration options.

## Installation

The module is automatically loaded when compiled with AzerothCore.

