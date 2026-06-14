/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

use zed_extension_api as zed;

struct BashppExtension;

impl zed::Extension for BashppExtension {
    fn new() -> Self {
        Self
    }

    fn language_server_command(
        &mut self,
        _language_server_id: &zed::LanguageServerId,
        worktree: &zed::Worktree,
    ) -> zed::Result<zed::Command> {
        let command = worktree
            .which("bpp-lsp")
            .ok_or_else(|| "bpp-lsp must be installed and available on PATH".to_string())?;

        Ok(zed::Command {
            command,
            args: vec!["--stdio".to_string()],
            env: Default::default(),
        })
    }
}

zed::register_extension!(BashppExtension);
