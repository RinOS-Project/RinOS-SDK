#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Build and package the SDK's standalone C consumer examples."""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path


EXAMPLES = {
    "hello": {
        "dependencies": ["rincrt"],
        "imports": [("rincrt", "rin_log")],
        "capabilities": [],
        "gui": False,
        "description": "Hello consumer using the public RinOS SDK.",
    },
    "process": {
        "dependencies": ["rincrt", "rinbase"],
        "imports": [
            ("rinbase", "rin_process_spawn_v1"),
            ("rinbase", "rin_process_wait_v1"),
            ("rinbase", "rin_object_close_v1"),
        ],
        "capabilities": ["process-control"],
        "gui": False,
        "description": "Least-privilege process spawn and wait consumer.",
    },
    "channel": {
        "dependencies": ["rincrt", "rinbase", "rinipc"],
        "imports": [
            ("rinipc", "rin_channel_create_v1"),
            ("rinipc", "rin_channel_send_v1"),
            ("rinipc", "rin_channel_receive_v1"),
            ("rinbase", "rin_object_close_v1"),
        ],
        "capabilities": [],
        "gui": False,
        "description": "Bounded channel round-trip consumer.",
    },
    "file_portal": {
        "dependencies": ["rincrt", "rinbase", "rinipc", "rinfs"],
        "imports": [
            ("rinfs", "rin_fs_portal_open_v1"),
            ("rinfs", "rin_file_read_v1"),
            ("rinfs", "rin_file_close_v1"),
            ("rincrt", "strtoull"),
            ("rincrt", "__errno_location"),
        ],
        "capabilities": ["file-portal"],
        "gui": False,
        "description": "Read-only durable File Portal grant consumer.",
    },
    "window": {
        "dependencies": ["rincrt", "rinruntime"],
        "imports": [
            ("rinruntime", "rin_window_create_v1"),
            ("rinruntime", "rin_window_next_event_v1"),
            ("rinruntime", "rin_window_close_v1"),
        ],
        "capabilities": ["gui-control"],
        "gui": True,
        "description": "Window creation and event consumer.",
    },
    "media": {
        "dependencies": ["rincrt", "rinbase", "rinmedia", "rinruntime"],
        "imports": [
            ("rinmedia", "rin_media_open_v1"),
            ("rinmedia", "rin_image_get_info_v1"),
            ("rinmedia", "rin_image_decode_v1"),
            ("rinmedia", "rin_audio_get_info_v1"),
            ("rinmedia", "rin_audio_decode_v1"),
            ("rinmedia", "rin_video_decode_v1"),
            ("rinmedia", "rin_video_get_info_v1"),
            ("rinmedia", "rin_video_decode_frame_v1"),
            ("rinruntime", "rin_surface_create_v1"),
            ("rinruntime", "rin_surface_map_v1"),
            ("rinruntime", "rin_surface_unmap_v1"),
            ("rinruntime", "rin_surface_destroy_v1"),
            ("rinbase", "rin_object_close_v1"),
            ("rincrt", "printf"),
            ("rincrt", "puts"),
        ],
        "capabilities": [],
        "gui": False,
        "description": "Typed image, PCM audio, and video-frame consumer with explicit unsupported handling.",
    },
    "keyring": {
        "dependencies": ["rincrt", "rinbase"],
        "imports": [
            ("rinbase", "rin_service_find_system_slot"),
            ("rincrt", "printf"),
            ("rincrt", "puts"),
        ],
        "capabilities": [],
        "gui": False,
        "description": "Read-only authenticated Keyring service-status consumer.",
    },
    "ime_provider": {
        "dependencies": ["rincrt", "rinbase"],
        "imports": [
            ("rinbase", "rin_service_find_system_slot"),
            ("rinbase", "rin_service_should_stop"),
            ("rinbase", "rin_service_health"),
            ("rincrt", "rin_log"),
        ],
        "capabilities": [],
        "gui": False,
        "description": "Authenticated public IME Provider wire-protocol example.",
        "service_entry_points": [{"name": "ime", "path": "ime_provider.rin"}],
    },
    "explorer": {
        "dependencies": ["rincrt", "rinbase"],
        "imports": [
            ("rinbase", "rin_process_spawn_v1"),
            ("rinbase", "rin_process_wait_v1"),
            ("rinbase", "rin_object_close_v1"),
            ("rincrt", "puts"),
            ("rincrt", "printf"),
            ("rincrt", "strcmp"),
            ("rincrt", "strlen"),
        ],
        "capabilities": ["process-control"],
        "gui": False,
        "description": "Developer-only SDK example catalog and launcher.",
    },
}
RUNTIME_PACKAGE_PREFIX = "org.rinos.sdklib."


def run(command: list[str], timeout: int) -> None:
    print(" ".join(str(part) for part in command))
    try:
        completed = subprocess.run(command, check=False, timeout=timeout)
    except subprocess.TimeoutExpired as error:
        raise RuntimeError(f"command timed out: {command[0]}") from error
    if completed.returncode != 0:
        raise RuntimeError(f"command failed ({completed.returncode}): {command[0]}")


def tool_path(value: str, label: str) -> Path:
    path = Path(value).expanduser()
    if path.is_symlink() or not path.is_file():
        raise RuntimeError(f"{label} is missing or unsafe: {path}")
    return path.resolve(strict=True)


def toml_string(value: str) -> str:
    return json.dumps(value, ensure_ascii=False)


def package_manifest(name: str, package_id: str, title: str, version: str,
                     architecture: str, capabilities: list[str],
                     dependencies: list[str], payload_name: str) -> str:
    dependency_records = [
        "{name = " + toml_string(RUNTIME_PACKAGE_PREFIX + library) +
        ", min_version = " + toml_string(version) + ', max_version = ""}'
        for library in dependencies
    ]
    entry_points = [{"name": "main", "path": payload_name}]
    entry_points.extend(EXAMPLES[name].get("service_entry_points", []))
    encoded_entry_points = "[" + ", ".join(
        "{name = " + toml_string(entry["name"]) + ", path = " +
        toml_string(entry["path"]) + "}"
        for entry in entry_points
    ) + "]"
    return "\n".join([
        "[package]",
        f"package_id = {toml_string(package_id)}",
        f"display_name = {toml_string(title)}",
        f"version = {toml_string(version)}",
        f"architecture = {toml_string(architecture)}",
        'package_type = "application"',
        f"description = {toml_string(EXAMPLES[name]['description'])}",
        'license = "MIT"',
        'homepage = ""',
        'min_rinos_version = "0.2.0"',
        'max_rinos_version = ""',
        "dependencies = [" + ", ".join(dependency_records) + "]",
        "optional_dependencies = []",
        "conflicts = []",
        "provides = []",
        "entry_points = " + encoded_entry_points,
        'flags = ["security"]',
        "capabilities = [" + ", ".join(
            toml_string(value) for value in capabilities
        ) + "]",
        "",
        "[[files]]",
        f"path = {toml_string(payload_name)}",
        f"source = {toml_string('payload/' + payload_name)}",
        "",
    ])


def build(args: argparse.Namespace) -> None:
    target = args.target
    architecture = "i686" if target == "i686-unknown-rinos" else "x86_64"
    sdk_root = Path(args.sdk_root).expanduser().resolve(strict=True)
    include_root = sdk_root / "include"
    metadata_path = sdk_root / "pkg" / "rinsdk-v1.json"
    if not (include_root / "rin" / "sdk.h").is_file():
        raise RuntimeError("SDK root must contain include/rin/sdk.h")
    for dependency in ("ringpu/ringpu.h", "rinresource/loader.h"):
        if not (include_root / dependency).is_file():
            raise RuntimeError(
                f"SDK include tree is missing {dependency}; use the packaged SDK prefix"
            )
    metadata = json.loads(metadata_path.read_text(encoding="utf-8"))
    if target not in metadata.get("targets", []):
        raise RuntimeError("target is absent from the SDK package metadata")
    receipt = json.loads((sdk_root / "manifest.json").read_text(encoding="utf-8"))
    if not isinstance(receipt, dict) or receipt.get("schema") != 1 or \
            receipt.get("target") != target or \
            not isinstance(receipt.get("artifacts"), dict):
        raise RuntimeError("SDK install receipt does not match the selected target")
    libraries = metadata.get("libraries")
    if not isinstance(libraries, dict):
        raise RuntimeError("SDK runtime library metadata is malformed")
    libraries = dict(libraries)
    libraries["rinruntime"] = {"shared": "RinRuntime.rll"}
    rinruntime_rll = Path(args.rinruntime_rll).expanduser()
    if rinruntime_rll.is_symlink() or not rinruntime_rll.is_file() or \
            rinruntime_rll.name != "RinRuntime.rll":
        raise RuntimeError(
            "--rinruntime-rll must name a regular installed RinRuntime.rll"
        )
    rinruntime_rll = rinruntime_rll.resolve(strict=True)

    if not re.fullmatch(r"[0-9]+(?:\.[0-9]+){0,3}", args.runtime_version):
        raise RuntimeError("runtime version must be numeric")
    publisher_generation = args.publisher_generation
    if publisher_generation < 1 or publisher_generation > 0xFFFFFFFF:
        raise RuntimeError("publisher generation is outside the package ABI")

    rcc = tool_path(args.rcc, "rcc")
    rld = tool_path(args.rld, "rld")
    rinsign = tool_path(args.rinsign, "rinsign")
    rinpack = tool_path(args.rinpack, "rinpack")
    validator = tool_path(args.validate_with, "RIN v3 validator")
    rin_sign_key = tool_path(args.rin_sign_key, "RIN image sign key")
    rin_public_key = tool_path(args.rin_public_key, "RIN image trust key")
    publisher_key = tool_path(args.publisher_key, "package publisher key")
    publisher_public_key = tool_path(
        args.publisher_public_key, "package publisher public key"
    )

    output_requested = Path(args.output_dir).expanduser()
    if output_requested.is_symlink():
        raise RuntimeError("output directory must not be a symlink")
    output = output_requested.resolve()
    output.mkdir(parents=True, exist_ok=True)
    examples_root = Path(__file__).resolve().parent
    timeout = args.command_timeout

    with tempfile.TemporaryDirectory(prefix=".rinosdk-consumer-", dir=output) as temp:
        stage = Path(temp)
        for name, spec in EXAMPLES.items():
            package_id = f"org.rinos.sdk.{name.replace('_', '-')}"
            title = "RinOS SDK " + name.replace("_", " ").title()
            payload_name = name + ".rin"
            object_path = stage / (name + ".ro")
            image_path = stage / payload_name
            package_path = output / f"{package_id}-{architecture}.rpk"
            if package_path.exists() or package_path.is_symlink():
                raise RuntimeError(f"output package already exists: {package_path}")
            service_manifest = None
            if name == "ime_provider":
                service_manifest = output / "ime-provider-user-service.SVC"
                if service_manifest.exists() or service_manifest.is_symlink():
                    raise RuntimeError(
                        f"output service manifest already exists: {service_manifest}"
                    )

            dependency_names = spec["dependencies"]
            for library in dependency_names:
                if library not in libraries:
                    raise RuntimeError(f"SDK metadata is missing runtime library {library}")
            library_names = {
                library: libraries[library]["shared"]
                for library in dependency_names
            }
            for library, shared in library_names.items():
                if not isinstance(shared, str) or Path(shared).name != shared or \
                        not shared.endswith(".rll"):
                    raise RuntimeError(f"SDK runtime metadata is invalid for {library}")
                shared_path = rinruntime_rll if library == "rinruntime" else \
                    sdk_root / shared
                if shared_path.is_symlink() or not shared_path.is_file() or \
                        (library != "rinruntime" and
                         hashlib.sha256(shared_path.read_bytes()).hexdigest() !=
                         receipt["artifacts"].get(shared)):
                    raise RuntimeError(
                        f"SDK runtime artifact does not match its install receipt: {shared}"
                    )
            image_dependencies = [library_names[name] for name in dependency_names]
            imports = list(spec["imports"]) + [
                ("rincrt", symbol) for symbol in ("memcpy", "memset", "memcmp")
            ]

            run([
                str(rcc), "--target", target, "-std=c17", "-ffreestanding", "-c",
                "-I", str(include_root), "-o", str(object_path),
                str(examples_root / (name + ".c")),
            ], timeout)

            link_command = [
                str(rld), "--target", target,
                "--sign-profile", args.sign_profile,
                "--rinsign", str(rinsign),
                "--sign-key", str(rin_sign_key),
                "--public-key", str(rin_public_key),
            ]
            for dependency in image_dependencies:
                link_command.extend(("--dep", dependency))
            for provider, symbol in imports:
                link_command.extend((
                    "--import", f"{symbol}={library_names[provider]}@function"
                ))
            link_command.extend(("-o", str(image_path), str(object_path)))
            run(link_command, timeout)

            validation_command = [
                str(validator), "--kind", "executable",
                "--arch", "x86" if architecture == "i686" else "x86_64",
                "--trust-key", str(rin_public_key), "--exact-dependencies",
            ]
            for dependency in image_dependencies:
                validation_command.extend(("--require-dependency", dependency))
            validation_command.append("--exact-imports")
            for provider, symbol in imports:
                validation_command.extend((
                    "--require-import", f"{library_names[provider]}:{symbol}@function"
                ))
            validation_command.append(str(image_path))
            run(validation_command, timeout)

            package_stage = stage / (name + "-package")
            payload_root = package_stage / "payload"
            payload_root.mkdir(parents=True)
            shutil.copyfile(image_path, payload_root / payload_name)
            title = "RinOS SDK " + name.replace("_", " ").title()
            (package_stage / "package.toml").write_text(
                package_manifest(
                    name, package_id, title, args.runtime_version, architecture,
                    spec["capabilities"], dependency_names, payload_name,
                ),
                encoding="utf-8",
            )
            application = {
                "application_id": package_id,
                "display_name": title,
                "entry_point": payload_name,
                "description": spec["description"],
                "categories": ["development"],
                "mime_types": [],
                "gui": spec["gui"],
            }
            application_path = package_stage / "application.json"
            application_path.write_text(
                json.dumps(application, ensure_ascii=False, sort_keys=True) + "\n",
                encoding="utf-8",
            )
            run([
                str(rinpack), "create",
                "--manifest", str(package_stage / "package.toml"),
                "--application-metadata", str(application_path),
                "--output", str(package_path),
                "--sign-key", str(publisher_key),
                "--public-key", str(publisher_public_key),
                "--publisher-generation", str(publisher_generation),
            ], timeout)
            run([
                str(rinpack), "verify", "--package", str(package_path),
                "--public-key", str(publisher_public_key), "--json",
            ], timeout)
            if name == "ime_provider":
                assert service_manifest is not None
                service_manifest.write_text(
                    "\n".join([
                        "[Service]",
                        "Id=ime",
                        "Name=SDK IME Provider Example",
                        "Exec=/Apps/org.rinos.sdk.ime-provider/ime_provider.rin",
                        "Scope=user",
                        "Capabilities=none",
                        "AutoStart=login",
                        "Restart=always",
                        "BackoffMs=500",
                        "StopTimeoutMs=3000",
                        "MaxRestarts=0",
                        "CrashWindowMs=30000",
                        "HealthIntervalMs=5000",
                        "HealthTimeoutMs=20000",
                        "",
                    ]),
                    encoding="utf-8",
                )
                print(f"created example user-service manifest: {service_manifest}")
            print(f"created SDK consumer package: {package_path}")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--sdk-root", required=True,
                        help="one installed target prefix from package_rinsdk_v1.py")
    parser.add_argument("--rinruntime-rll", required=True,
                        help="signed RinRuntime.rll built with GUI support")
    parser.add_argument("--target", choices=(
        "i686-unknown-rinos", "x86_64-unknown-rinos",
    ), required=True)
    parser.add_argument("--output-dir", required=True)
    parser.add_argument("--rcc", required=True)
    parser.add_argument("--rld", required=True)
    parser.add_argument("--rinsign", required=True)
    parser.add_argument("--rinpack", required=True)
    parser.add_argument("--validate-with", required=True)
    parser.add_argument("--rin-sign-key", required=True)
    parser.add_argument("--rin-public-key", required=True)
    parser.add_argument("--publisher-key", required=True)
    parser.add_argument("--publisher-public-key", required=True)
    parser.add_argument("--sign-profile", choices=("debug", "release"),
                        default="debug")
    parser.add_argument("--runtime-version", default="1.0.0")
    parser.add_argument("--publisher-generation", type=int, default=1)
    parser.add_argument("--command-timeout", type=int, default=120)
    args = parser.parse_args()
    build(args)
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, RuntimeError, ValueError, KeyError,
            json.JSONDecodeError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        raise SystemExit(1)
