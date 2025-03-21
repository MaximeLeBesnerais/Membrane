import React, { useState, useEffect } from "react";
import {
  CssBaseline,
  Container,
  Typography,
  Box,
  Button,
  TextField,
  Grid,
  Paper,
  Snackbar,
  Alert,
  Tab,
  Tabs,
  Card,
  CardContent
} from "@mui/material";

import {
  ContentCopy,
  ContentPaste,
  Save,
  CreateNewFolder,
  NoteAdd,
  Link,
  Cloud,
  CloudUpload,
  Functions,
  Info,
} from "@mui/icons-material";

// Custom TabPanel component for tab content
function TabPanel({ children, value, index, ...other }) {
  return (
    <div
      role="tabpanel"
      hidden={value !== index}
      id={`function-tabpanel-${index}`}
      aria-labelledby={`function-tab-${index}`}
      {...other}
    >
      {value === index && <Box sx={{ p: 3 }}>{children}</Box>}
    </div>
  );
}

export const Demo = () => {
  // State for feedback message
  const [feedback, setFeedback] = useState({
    open: false,
    message: "",
    severity: "info",
  });

  // State for registered functions
  const [functions, setFunctions] = useState([]);

  // Tab state
  const [tabValue, setTabValue] = useState(0);

  // UI states for different functions
  const [clipboardText, setClipboardText] = useState("");
  const [customVfsName, setCustomVfsName] = useState("");
  const [persistenceDir, setPersistenceDir] = useState("");
  const [vfsToSave, setVfsToSave] = useState("");
  const [fileContent, setFileContent] = useState("");
  const [filePath, setFilePath] = useState("");
  const [vfsForFile, setVfsForFile] = useState("");
  const [vfsFilePath, setVfsFilePath] = useState("");
  const [externalUrl, setExternalUrl] = useState("https://");

  // Load available functions on mount
  useEffect(() => {
    listFunctions();
  }, []);

  // Handle tab change
  const handleTabChange = (event, newValue) => {
    setTabValue(newValue);
  };

  // Shows a feedback message
  const showFeedback = (message, severity = "success") => {
    setFeedback({
      open: true,
      message,
      severity,
    });
  };

  // Handles Snackbar close
  const handleSnackbarClose = () => {
    setFeedback({ ...feedback, open: false });
  };

  // List available functions
  const listFunctions = async () => {
    try {
      const result = await window.membrane_listFunctions();

      if (result.status === "success") {
        setFunctions(result.data);
        showFeedback("Successfully retrieved functions", "info");
      } else {
        showFeedback(`Error: ${result.message}`, "error");
      }
    } catch (error) {
      showFeedback(`Error: ${error.message}`, "error");
    }
  };

  // Clipboard operations
  const readClipboard = async () => {
    try {
      const result = await window.membrane_readClipboard(JSON.stringify([]));

      if (result.status === "success") {
        setClipboardText(result.data);
        showFeedback("Clipboard content read successfully");
      } else {
        showFeedback(`Error: ${result.message}`, "error");
      }
    } catch (error) {
      showFeedback(`Error: ${error.message}`, "error");
    }
  };

  const writeClipboard = async () => {
    try {
      const args = clipboardText;
      const result = await window.membrane_writeClipboard(args);

      if (result.status === "success") {
        showFeedback("Content written to clipboard");
      } else {
        showFeedback(`Error: ${result.message}`, "error");
      }
    } catch (error) {
      showFeedback(`Error: ${error.message}`, "error");
    }
  };

  // VFS operations
  const createCustomVfs = async () => {
    if (!customVfsName) {
      showFeedback("VFS name is required", "warning");
      return;
    }

    try {
      const result = persistenceDir != "" ? await window.membrane_createCustomVfs(customVfsName, persistenceDir)
        : await window.membrane_createCustomVfs(customVfsName);

      if (result.status === "success") {
        showFeedback(`VFS "${customVfsName}" created successfully`);
        setCustomVfsName("");
        setPersistenceDir("");
      } else {
        showFeedback(`Error: ${result.message}`, "error");
      }
    } catch (error) {
      showFeedback(`Error: ${error.message}`, "error");
    }
  };

  const saveVfsToDisk = async () => {
    if (!vfsToSave) {
      showFeedback("VFS name is required", "warning");
      return;
    }

    try {
      // Create a single-element array with the VFS name
      const result = await window.membrane_saveVfsToDisk(vfsToSave);

      if (result.status === "success") {
        showFeedback(`VFS "${vfsToSave}" saved to disk successfully`);
        setVfsToSave("");
      } else {
        showFeedback(`Error: ${result.message}`, "error");
      }
    } catch (error) {
      showFeedback(`Error: ${error.message}`, "error");
    }
  };

  const saveAllVfsToDisk = async () => {
    try {
      // Pass an empty array when no parameters are needed
      const result = await window.membrane_saveAllVfsToDisk();

      if (result.status === "success") {
        showFeedback("All VFS instances saved to disk successfully");
      } else {
        showFeedback(`Error: ${result.message}`, "error");
      }
    } catch (error) {
      showFeedback(`Error: ${error.message}`, "error");
    }
  };

  const addFileToVfs = async () => {
    if (!vfsForFile || !vfsFilePath || !fileContent) {
      showFeedback("VFS name, file path, and content are required", "warning");
      return;
    }

    try {
      const result = await window.membrane_addFileToVfs(vfsForFile, vfsFilePath, fileContent);

      if (result.status === "success") {
        showFeedback(`File added to VFS "${vfsForFile}" successfully`);
        setVfsFilePath("");
        setFileContent("");
      } else {
        showFeedback(`Error: ${result.message}`, "error");
      }
    } catch (error) {
      showFeedback(`Error: ${error.message}`, "error");
    }
  };

  // File system operations
  const saveFile = async () => {
    if (!filePath || !fileContent) {
      showFeedback("File path and content are required", "warning");
      return;
    }

    try {
      const result = await window.membrane_saveFile(filePath, fileContent);

      if (result.status === "success") {
        showFeedback(`File saved to "${filePath}" successfully`);
        setFilePath("");
        setFileContent("");
      } else {
        showFeedback(`Error: ${result.message}`, "error");
      }
    } catch (error) {
      showFeedback(`Error: ${error.message}`, "error");
    }
  };

  // External URL handling
  const openExternalUrl = async () => {
    if (!externalUrl) {
      showFeedback("URL is required", "warning");
      return;
    }

    try {
      // Create a single-element array with the URL
      const result = await window.membrane_openExternalUrl(externalUrl);

      if (result.status === "success") {
        showFeedback(`Opened URL in external browser`);
      } else {
        showFeedback(`Error: ${result.message}`, "error");
      }
    } catch (error) {
      showFeedback(`Error: ${error.message}`, "error");
    }
  };

  // UI Components
  const renderHeader = () => (
    <Paper
      elevation={4}
      sx={{ p: 3, borderRadius: 2, backgroundColor: "#2c2c2c", mb: 3 }}
    >
      <Typography
        variant="h4"
        align="center"
        sx={{
          mb: 2,
          background: "linear-gradient(to right, #42a5f5, #7e57c2)",
          WebkitBackgroundClip: "text",
          WebkitTextFillColor: "transparent",
          fontWeight: "bold",
        }}
      >
        Membrane Function Explorer
      </Typography>

      <Typography color="gray" align="center" mb={3}>
        Interact with Membrane's core C++ functions through JavaScript
      </Typography>

      <Grid container spacing={2} justifyContent="center" mb={3}>
        <Grid item>
          <Button
            variant="contained"
            color="primary"
            startIcon={<Functions />}
            onClick={listFunctions}
            sx={{
              background: "linear-gradient(to right, #42a5f5, #7e57c2)",
              color: "white",
              "&:hover": { opacity: 0.9 },
            }}
          >
            Refresh Functions
          </Button>
        </Grid>
      </Grid>

      {renderFunctionsList()}
    </Paper>
  );

  const renderFunctionsList = () => (
    functions.length > 0 && (
      <Box mb={3}>
        <Typography variant="h6" color="white" gutterBottom>
          Available Functions:
        </Typography>
        <Grid container spacing={1}>
          {functions.map((func, index) => (
            <Grid item xs={12} sm={6} md={4} key={index}>
              <Card
                variant="outlined"
                sx={{ backgroundColor: "#3a3a3a", color: "white" }}
              >
                <CardContent>
                  <Typography variant="body2" color="#42a5f5">
                    {func}
                  </Typography>
                </CardContent>
              </Card>
            </Grid>
          ))}
        </Grid>
      </Box>
    )
  );

  const renderTabs = () => (
    <Box sx={{ borderBottom: 1, borderColor: "divider" }}>
      <Tabs
        value={tabValue}
        onChange={handleTabChange}
        variant="scrollable"
        scrollButtons="auto"
        textColor="primary"
        indicatorColor="primary"
        sx={{
          "& .MuiTab-root": { color: "white" },
          "& .Mui-selected": { color: "#42a5f5" },
        }}
      >
        <Tab
          icon={<ContentPaste />}
          iconPosition="start"
          label="Clipboard"
        />
        <Tab
          icon={<CreateNewFolder />}
          iconPosition="start"
          label="VFS"
        />
        <Tab icon={<Save />} iconPosition="start" label="Files" />
        <Tab
          icon={<Link />}
          iconPosition="start"
          label="External URL"
        />
      </Tabs>
    </Box>
  );

  const renderClipboardTab = () => (
    <TabPanel value={tabValue} index={0}>
      <Paper elevation={2} sx={{ p: 3, backgroundColor: "#3a3a3a" }}>
        <Typography variant="h6" color="white" gutterBottom>
          Clipboard Operations
        </Typography>
        <TextField
          fullWidth
          label="Clipboard Content"
          variant="outlined"
          value={clipboardText}
          onChange={(e) => setClipboardText(e.target.value)}
          multiline
          rows={4}
          margin="normal"
          InputProps={{
            style: { color: "white" },
          }}
          InputLabelProps={{
            style: { color: "#aaa" },
          }}
          sx={{
            mb: 2,
            "& .MuiOutlinedInput-root": {
              "& fieldset": { borderColor: "#555" },
              "&:hover fieldset": { borderColor: "#777" },
              "&.Mui-focused fieldset": { borderColor: "#42a5f5" },
            },
          }}
        />
        <Grid container spacing={2}>
          <Grid item>
            <Button
              variant="contained"
              color="primary"
              startIcon={<ContentPaste />}
              onClick={readClipboard}
            >
              Read from Clipboard
            </Button>
          </Grid>
          <Grid item>
            <Button
              variant="contained"
              color="secondary"
              startIcon={<ContentCopy />}
              onClick={writeClipboard}
            >
              Write to Clipboard
            </Button>
          </Grid>
        </Grid>
      </Paper>
    </TabPanel>
  );

  const renderCreateVfsSection = () => (
    <Paper
      elevation={2}
      sx={{ p: 3, backgroundColor: "#3a3a3a", mb: 3 }}
    >
      <Typography variant="h6" color="white" gutterBottom>
        Create Custom VFS
      </Typography>
      <Grid container spacing={2}>
        <Grid item xs={12} sm={6}>
          <TextField
            fullWidth
            label="VFS Name"
            variant="outlined"
            value={customVfsName}
            onChange={(e) => setCustomVfsName(e.target.value)}
            margin="normal"
            InputProps={{
              style: { color: "white" },
            }}
            InputLabelProps={{
              style: { color: "#aaa" },
            }}
            sx={{
              "& .MuiOutlinedInput-root": {
                "& fieldset": { borderColor: "#555" },
                "&:hover fieldset": { borderColor: "#777" },
                "&.Mui-focused fieldset": { borderColor: "#42a5f5" },
              },
            }}
          />
        </Grid>
        <Grid item xs={12} sm={6}>
          <TextField
            fullWidth
            label="Persistence Directory (optional)"
            variant="outlined"
            value={persistenceDir}
            onChange={(e) => setPersistenceDir(e.target.value)}
            margin="normal"
            InputProps={{
              style: { color: "white" },
            }}
            InputLabelProps={{
              style: { color: "#aaa" },
            }}
            sx={{
              "& .MuiOutlinedInput-root": {
                "& fieldset": { borderColor: "#555" },
                "&:hover fieldset": { borderColor: "#777" },
                "&.Mui-focused fieldset": { borderColor: "#42a5f5" },
              },
            }}
          />
        </Grid>
      </Grid>
      <Button
        variant="contained"
        color="primary"
        startIcon={<CreateNewFolder />}
        onClick={createCustomVfs}
        sx={{ mt: 2 }}
      >
        Create VFS
      </Button>
    </Paper>
  );

  const renderSaveVfsSection = () => (
    <Paper
      elevation={2}
      sx={{ p: 3, backgroundColor: "#3a3a3a", mb: 3 }}
    >
      <Typography variant="h6" color="white" gutterBottom>
        Save VFS to Disk
      </Typography>
      <TextField
        fullWidth
        label="VFS Name"
        variant="outlined"
        value={vfsToSave}
        onChange={(e) => setVfsToSave(e.target.value)}
        margin="normal"
        InputProps={{
          style: { color: "white" },
        }}
        InputLabelProps={{
          style: { color: "#aaa" },
        }}
        sx={{
          mb: 2,
          "& .MuiOutlinedInput-root": {
            "& fieldset": { borderColor: "#555" },
            "&:hover fieldset": { borderColor: "#777" },
            "&.Mui-focused fieldset": { borderColor: "#42a5f5" },
          },
        }}
      />
      <Grid container spacing={2}>
        <Grid item>
          <Button
            variant="contained"
            color="primary"
            startIcon={<Save />}
            onClick={saveVfsToDisk}
          >
            Save VFS to Disk
          </Button>
        </Grid>
        <Grid item>
          <Button
            variant="contained"
            color="secondary"
            startIcon={<CloudUpload />}
            onClick={saveAllVfsToDisk}
          >
            Save All VFS
          </Button>
        </Grid>
      </Grid>
    </Paper>
  );

  const renderAddFileToVfsSection = () => (
    <Paper elevation={2} sx={{ p: 3, backgroundColor: "#3a3a3a" }}>
      <Typography variant="h6" color="white" gutterBottom>
        Add File to VFS
      </Typography>
      <Grid container spacing={2}>
        <Grid item xs={12} sm={6}>
          <TextField
            fullWidth
            label="VFS Name"
            variant="outlined"
            value={vfsForFile}
            onChange={(e) => setVfsForFile(e.target.value)}
            margin="normal"
            InputProps={{
              style: { color: "white" },
            }}
            InputLabelProps={{
              style: { color: "#aaa" },
            }}
            sx={{
              "& .MuiOutlinedInput-root": {
                "& fieldset": { borderColor: "#555" },
                "&:hover fieldset": { borderColor: "#777" },
                "&.Mui-focused fieldset": { borderColor: "#42a5f5" },
              },
            }}
          />
        </Grid>
        <Grid item xs={12} sm={6}>
          <TextField
            fullWidth
            label="File Path"
            variant="outlined"
            value={vfsFilePath}
            onChange={(e) => setVfsFilePath(e.target.value)}
            margin="normal"
            InputProps={{
              style: { color: "white" },
            }}
            InputLabelProps={{
              style: { color: "#aaa" },
            }}
            sx={{
              "& .MuiOutlinedInput-root": {
                "& fieldset": { borderColor: "#555" },
                "&:hover fieldset": { borderColor: "#777" },
                "&.Mui-focused fieldset": { borderColor: "#42a5f5" },
              },
            }}
          />
        </Grid>
      </Grid>
      <TextField
        fullWidth
        label="File Content"
        variant="outlined"
        value={fileContent}
        onChange={(e) => setFileContent(e.target.value)}
        multiline
        rows={4}
        margin="normal"
        InputProps={{
          style: { color: "white" },
        }}
        InputLabelProps={{
          style: { color: "#aaa" },
        }}
        sx={{
          mb: 2,
          "& .MuiOutlinedInput-root": {
            "& fieldset": { borderColor: "#555" },
            "&:hover fieldset": { borderColor: "#777" },
            "&.Mui-focused fieldset": { borderColor: "#42a5f5" },
          },
        }}
      />
      <Button
        variant="contained"
        color="primary"
        startIcon={<NoteAdd />}
        onClick={addFileToVfs}
      >
        Add File to VFS
      </Button>
    </Paper>
  );

  const renderVfsTab = () => (
    <TabPanel value={tabValue} index={1}>
      {renderCreateVfsSection()}
      {renderSaveVfsSection()}
      {renderAddFileToVfsSection()}
    </TabPanel>
  );

  const renderFilesTab = () => (
    <TabPanel value={tabValue} index={2}>
      <Paper elevation={2} sx={{ p: 3, backgroundColor: "#3a3a3a" }}>
        <Typography variant="h6" color="white" gutterBottom>
          Save File to Disk
        </Typography>
        <TextField
          fullWidth
          label="File Path"
          variant="outlined"
          value={filePath}
          onChange={(e) => setFilePath(e.target.value)}
          margin="normal"
          InputProps={{
            style: { color: "white" },
          }}
          InputLabelProps={{
            style: { color: "#aaa" },
          }}
          sx={{
            "& .MuiOutlinedInput-root": {
              "& fieldset": { borderColor: "#555" },
              "&:hover fieldset": { borderColor: "#777" },
              "&.Mui-focused fieldset": { borderColor: "#42a5f5" },
            },
          }}
        />
        <TextField
          fullWidth
          label="File Content"
          variant="outlined"
          value={fileContent}
          onChange={(e) => setFileContent(e.target.value)}
          multiline
          rows={4}
          margin="normal"
          InputProps={{
            style: { color: "white" },
          }}
          InputLabelProps={{
            style: { color: "#aaa" },
          }}
          sx={{
            mb: 2,
            "& .MuiOutlinedInput-root": {
              "& fieldset": { borderColor: "#555" },
              "&:hover fieldset": { borderColor: "#777" },
              "&.Mui-focused fieldset": { borderColor: "#42a5f5" },
            },
          }}
        />
        <Button
          variant="contained"
          color="primary"
          startIcon={<Save />}
          onClick={saveFile}
        >
          Save to Disk
        </Button>
      </Paper>
    </TabPanel>
  );

  const renderExternalUrlTab = () => (
    <TabPanel value={tabValue} index={3}>
      <Paper elevation={2} sx={{ p: 3, backgroundColor: "#3a3a3a" }}>
        <Typography variant="h6" color="white" gutterBottom>
          Open External URL
        </Typography>
        <TextField
          fullWidth
          label="URL"
          variant="outlined"
          value={externalUrl}
          onChange={(e) => setExternalUrl(e.target.value)}
          margin="normal"
          InputProps={{
            style: { color: "white" },
          }}
          InputLabelProps={{
            style: { color: "#aaa" },
          }}
          sx={{
            mb: 2,
            "& .MuiOutlinedInput-root": {
              "& fieldset": { borderColor: "#555" },
              "&:hover fieldset": { borderColor: "#777" },
              "&.Mui-focused fieldset": { borderColor: "#42a5f5" },
            },
          }}
        />
        <Button
          variant="contained"
          color="primary"
          startIcon={<Link />}
          onClick={openExternalUrl}
        >
          Open in Browser
        </Button>
      </Paper>
    </TabPanel>
  );

  const renderFooter = () => (
    <Typography textAlign="center" color="gray" variant="caption">
      Membrane - A C++ and Web Tech Interface © 2025 Maxime Le Besnerais
    </Typography>
  );

  const renderFeedbackSnackbar = () => (
    <Snackbar
      open={feedback.open}
      autoHideDuration={6000}
      onClose={handleSnackbarClose}
      anchorOrigin={{ vertical: "bottom", horizontal: "center" }}
    >
      <Alert
        onClose={handleSnackbarClose}
        severity={feedback.severity}
        sx={{ width: "100%" }}
      >
        {feedback.message}
      </Alert>
    </Snackbar>
  );

  return (
    <>
      <CssBaseline />
      <Box
        sx={{
          minHeight: "100vh",
          display: "flex",
          flexDirection: "column",
          background: "linear-gradient(to bottom right, #1e1e1e, #121212)",
          p: 2,
        }}
      >
        <Container maxWidth="md">
          {renderHeader()}
          
          <Paper
            elevation={4}
            sx={{ p: 3, borderRadius: 2, backgroundColor: "#2c2c2c", mb: 3 }}
          >
            {renderTabs()}
            {renderClipboardTab()}
            {renderVfsTab()}
            {renderFilesTab()}
            {renderExternalUrlTab()}
          </Paper>

          {renderFooter()}
        </Container>

        {renderFeedbackSnackbar()}
      </Box>
    </>
  );
};

export default Demo;