import React, { useState, useEffect } from "react";
import { Copy, Check, AlertCircle, Info, KeyRound, Sun, Moon, Palette, Settings } from "lucide-react";
import "./index.css";

// Theme configurations
const themes = {
    dark: {
        name: "Dark",
        icon: <Moon size={16} />,
        colors: {
            bgDark: "#131b2c",
            panelBg: "#1c2336",
            textColor: "#e4e6eb",
            inputBg: "#2a3347",
            inputBorder: "#3a4257",
            yellowButton: "#f4c12e",
            yellowText: "#f4c12e",
            warningBg: "rgba(244, 156, 12, 0.1)",
            warningBorder: "#f09c0c",
            warningText: "#f09c0c",
            errorColor: "#f44336",
            grayText: "#9ba1ad"
        }
    },
    light: {
        name: "Light",
        icon: <Sun size={16} />,
        colors: {
            bgDark: "#f5f7fa",
            panelBg: "#ffffff",
            textColor: "#333333",
            inputBg: "#f0f2f5",
            inputBorder: "#d0d7de",
            yellowButton: "#e6a700",
            yellowText: "#d99a00",
            warningBg: "rgba(244, 156, 12, 0.1)",
            warningBorder: "#f09c0c",
            warningText: "#d58500",
            errorColor: "#d73a49",
            grayText: "#6e7781"
        }
    },
    custom: {
        name: "Custom",
        icon: <Palette size={16} />,
        colors: {
            bgDark: "#131b2c",
            panelBg: "#1c2336",
            textColor: "#e4e6eb",
            inputBg: "#2a3347",
            inputBorder: "#3a4257",
            yellowButton: "#f4c12e",
            yellowText: "#f4c12e",
            warningBg: "rgba(244, 156, 12, 0.1)",
            warningBorder: "#f09c0c",
            warningText: "#f09c0c",
            errorColor: "#f44336",
            grayText: "#9ba1ad"
        }
    }
};

// Settings Modal Component
const SettingsModal = ({ isOpen, onClose, activeTheme, setActiveTheme, customColors, setCustomColors }) => {
    const [tempCustomColors, setTempCustomColors] = useState({...customColors});
    const [selectedTab, setSelectedTab] = useState(activeTheme);

    useEffect(() => {
        if (isOpen) {
            setTempCustomColors({...customColors});
            setSelectedTab(activeTheme);
        }
    }, [isOpen, customColors, activeTheme]);

    const handleColorChange = (key, value) => {
        setTempCustomColors(prev => ({
            ...prev,
            [key]: value
        }));
    };

    const saveSettings = () => {
        if (selectedTab === 'custom') {
            setCustomColors(tempCustomColors);
        }
        setActiveTheme(selectedTab);
        onClose();
    };

    if (!isOpen) return null;

    return (
        <div className={`modal-overlay ${isOpen ? '' : 'hidden'}`}>
            <div className="settings-modal">
                <div className="modal-header">
                    <h2>Theme Settings</h2>
                    <button className="close-button" onClick={onClose}>×</button>
                </div>

                <div className="modal-tabs">
                    {Object.keys(themes).map(theme => (
                        <button
                            key={theme}
                            className={`tab-button ${selectedTab === theme ? 'active' : ''}`}
                            onClick={() => setSelectedTab(theme)}
                        >
                            <span className="tab-icon">{themes[theme].icon}</span>
                            {themes[theme].name}
                        </button>
                    ))}
                </div>

                <div className="modal-content">
                    {selectedTab === 'dark' && (
                        <div className="theme-preview dark-preview">
                            <h3>Dark Theme</h3>
                            <p>Dark mode theme with blue and yellow accent colors.</p>
                            <div className="shortcut-hint">Keyboard shortcut: Alt+D</div>
                        </div>
                    )}

                    {selectedTab === 'light' && (
                        <div className="theme-preview light-preview">
                            <h3>Light Theme</h3>
                            <p>Light mode theme with white background and gold accent colors.</p>
                            <div className="shortcut-hint">Keyboard shortcut: Alt+L</div>
                        </div>
                    )}

                    {selectedTab === 'custom' && (
                        <div className="custom-theme-editor">
                            <h3>Custom Theme</h3>
                            <div className="shortcut-hint">Keyboard shortcut: Alt+C</div>

                            <div className="color-grid">
                                {Object.entries(tempCustomColors).map(([key, value]) => (
                                    <div key={key} className="color-field">
                                        <label>{key.replace(/([A-Z])/g, ' $1').charAt(0).toUpperCase() + key.replace(/([A-Z])/g, ' $1').slice(1)}</label>
                                        <div className="color-inputs">
                                            <input
                                                type="color"
                                                value={value}
                                                onChange={(e) => handleColorChange(key, e.target.value)}
                                                className="color-picker"
                                            />
                                            <input
                                                type="text"
                                                value={value}
                                                onChange={(e) => handleColorChange(key, e.target.value)}
                                                className="color-text"
                                            />
                                        </div>
                                    </div>
                                ))}
                            </div>
                        </div>
                    )}
                </div>

                <div className="modal-footer">
                    <button className="cancel-button" onClick={onClose}>Cancel</button>
                    <button className="save-button" onClick={saveSettings}>Save Changes</button>
                </div>
            </div>
        </div>
    );
};

// Main component
const SSHKeyGenerator = () => {
    const [form, setForm] = useState({
        keyType: "RSA",
        keySize: "2048",
        email: "",
        comment: "",
        passphrase: "",
    });
    const [loading, setLoading] = useState(false);
    const [generatedKey, setGeneratedKey] = useState("");
    const [error, setError] = useState(null);
    const [copied, setCopied] = useState(false);
    const [strength, setStrength] = useState("Weak");
    const [activeTheme, setActiveTheme] = useState("dark");
    const [customColors, setCustomColors] = useState({...themes.custom.colors});
    const [isSettingsOpen, setIsSettingsOpen] = useState(false);
    const [warningVisible, setWarningVisible] = useState(true);
    const [privateKeyLocation, setPrivateKeyLocation] = useState("");

    // Key mappings for theme shortcuts
    const keyMappings = {
        "alt+d": "dark",
        "alt+l": "light",
        "alt+c": "custom",
        "alt+s": "settings" // New shortcut to open settings
    };

    // Apply theme colors to CSS variables
    useEffect(() => {
        const currentTheme = activeTheme === "custom" ?
            { ...themes.custom, colors: customColors } :
            themes[activeTheme];

        Object.entries(currentTheme.colors).forEach(([key, value]) => {
            // Convert camelCase to kebab-case for CSS variables
            const cssVarName = key.replace(/([A-Z])/g, "-$1").toLowerCase();
            document.documentElement.style.setProperty(`--${cssVarName}`, value);
        });

        // Save to localStorage
        localStorage.setItem("activeTheme", activeTheme);
        localStorage.setItem("customTheme", JSON.stringify(customColors));
    }, [activeTheme, customColors]);

    // Load saved theme preferences
    useEffect(() => {
        const savedTheme = localStorage.getItem("activeTheme");
        const savedCustomTheme = localStorage.getItem("customTheme");

        if (savedTheme) {
            setActiveTheme(savedTheme);
        }

        if (savedCustomTheme) {
            try {
                setCustomColors(JSON.parse(savedCustomTheme));
            } catch (e) {
                console.error("Error parsing saved custom theme", e);
            }
        }
    }, []);

    // Keyboard shortcut listener
    useEffect(() => {
        const handleKeyDown = (e) => {
            // Check for theme shortcuts (Alt + key)
            if (e.altKey) {
                const shortcut = `alt+${e.key.toLowerCase()}`;
                if (keyMappings[shortcut]) {
                    e.preventDefault();

                    if (keyMappings[shortcut] === "settings") {
                        setIsSettingsOpen(true);
                    } else {
                        setActiveTheme(keyMappings[shortcut]);
                    }
                }
            }
        };

        window.addEventListener("keydown", handleKeyDown);
        return () => window.removeEventListener("keydown", handleKeyDown);
    }, []);

    // Calculate passphrase strength
    useEffect(() => {
        if (!form.passphrase) {
            setStrength("Weak");
            return;
        }

        let score = 0;
        if (form.passphrase.length > 8) score += 1;
        if (form.passphrase.length > 12) score += 1;
        if (/[A-Z]/.test(form.passphrase)) score += 1;
        if (/[0-9]/.test(form.passphrase)) score += 1;
        if (/[^A-Za-z0-9]/.test(form.passphrase)) score += 1;

        if (score <= 2) setStrength("Weak");
        else if (score <= 3) setStrength("Medium");
        else setStrength("Strong");
    }, [form.passphrase]);

    const handleChange = (e) => {
        setForm({ ...form, [e.target.name]: e.target.value });
        if (error && e.target.name === "email" && e.target.value.trim() !== "") {
            setError(null);
        }
    };

    const generateKey = () => {
        setLoading(true);
        setError(null);
    
        if (form.email.trim() === "") {
            setError("Email is required");
            setLoading(false);
            return;
        }
    

        window.generateSSHKey(form.keyType, form.keySize, form.email, form.comment, form.passphrase)
            .then(result => {
                if (result.status === "success") {
                    setGeneratedKey(result.publicKey);
                    setPrivateKeyLocation(result.privateKey);
                } else {
                    setError(result.message || "An error occurred while generating the key");
                }
                setLoading(false);
            })
            .catch(err => {
                setError("Failed to generate key: " + (err.message || "Unknown error"));
                setLoading(false);
            });
    };

    const copyToClipboard = () => {
        navigator.clipboard.writeText(generatedKey).then(() => {
            setCopied(true);
            setTimeout(() => setCopied(false), 1500);
        });
    };

    // Map for fixed key sizes based on key type
    const keyTypeSizes = {
        "ED25519": "256 bits",
        "ECDSA": "256 bits",
        "DSA": "1024 bits"
    };

    return (
        <div className="ssh-generator">
            {/* Header with icon and settings button */}
            <div className="header-container">
                <div className="header">
                    <span className="icon">
                        <KeyRound size={32} />
                    </span>
                    <h1>SSH Key Generator</h1>
                </div>

                <button
                    className="settings-button"
                    onClick={() => setIsSettingsOpen(true)}
                    title="Theme Settings (Alt+S)"
                >
                    <Settings size={20} />
                </button>
            </div>

            {/* Settings Modal */}
            <SettingsModal
                isOpen={isSettingsOpen}
                onClose={() => setIsSettingsOpen(false)}
                activeTheme={activeTheme}
                setActiveTheme={setActiveTheme}
                customColors={customColors}
                setCustomColors={setCustomColors}
            />

            <div className="form-container">
                <div className="form-row">
                    {/* Left column - Key Type */}
                    <div className="form-group form-group-half">
                        <label>Key Type</label>
                        <select name="keyType" value={form.keyType} onChange={handleChange} className="form-select">
                            <option value="RSA">RSA</option>
                            <option value="ED25519">ED25519</option>
                            <option value="ECDSA">ECDSA</option>
                            <option value="DSA">DSA</option>
                        </select>
                    </div>

                    {/* Right column - Key Size */}
                    <div className="form-group form-group-half">
                        <label>Key Size</label>
                        {form.keyType === "RSA" ? (
                            <select name="keySize" value={form.keySize} onChange={handleChange} className="form-select">
                                <option value="2048">2048 bits</option>
                                <option value="4096">4096 bits</option>
                                <option value="8192">8192 bits</option>
                            </select>
                        ) : (
                            <div className="form-fixed-value">{keyTypeSizes[form.keyType]}</div>
                        )}
                    </div>
                </div>

                <div className="form-group">
                    <label>Email Address *</label>
                    <input
                        type="email"
                        name="email"
                        value={form.email}
                        onChange={handleChange}
                        placeholder="your.email@example.com"
                        className="form-input"
                    />
                </div>

                <div className="form-group">
                    <label>Comment (Optional)</label>
                    <input
                        type="text"
                        name="comment"
                        value={form.comment}
                        onChange={handleChange}
                        className="form-input"
                    />
                </div>

                <div className="form-group">
                    <div className="label-with-hint">
                        <label>Passphrase (Optional)</label>
                        <span className="recommended-tag">✓ Recommended for security</span>
                    </div>
                    <input
                        type="password"
                        name="passphrase"
                        value={form.passphrase}
                        onChange={handleChange}
                        className="form-input"
                    />
                </div>

                <div className="strength-container">
                    <div className="strength-label">Strength</div>
                    <div className={`strength-value ${strength.toLowerCase()}`}>{strength}</div>
                </div>

                {error && (
                    <div className="error-message">
                        <AlertCircle size={16} />
                        <span>{error}</span>
                    </div>
                )}

                <button
                    className="generate-button"
                    onClick={generateKey}
                    disabled={loading}
                >
                    {loading ? "Generating..." : "Generate SSH Key"}
                </button>

                <div className={`warning-message ${form.keyType === "RSA" && form.keySize === "2048" ? '' : 'hidden'}`}>
                    <Info size={16} />
                    <span>Note: 2048-bit RSA keys are increasingly considered insufficient for long-term security. Consider using ED25519 or 4096-bit RSA keys instead.</span>
                </div>

                <div className={`key-output ${generatedKey ? 'has-key' : 'no-key'}`}>
                    {generatedKey ? (
                        <>
                            <div className="generated-key-text">
                                {generatedKey}
                            </div>
                            <button
                                className="copy-button"
                                onClick={copyToClipboard}
                            >
                                {copied ? <Check size={16} /> : <Copy size={16} />}
                                {copied ? "Copied!" : "Copy"}
                            </button>
                        </>
                    ) : (
                        <div className="key-placeholder">Generated SSH key will appear here</div>
                    )}
                </div>
                <div className="key-output">
                    {privateKeyLocation ? (
                        <>
                            <div className="generated-key-text">
                                {privateKeyLocation}
                            </div>
                        </>
                    ) : (
                        <div className="key-placeholder">Private Key Location will appear here</div>
                    )}
                </div>
            </div>
        </div>
    );
};

export function App() {
    return (
        <div className="app">
            <SSHKeyGenerator />
        </div>
    );
}

export default App;