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
  List,
  ListItem,
  ListItemText,
  Card,
  CardContent,
  Divider,
} from "@mui/material";

import {
  ContentPaste,
  Save,
  CreateNewFolder,
  NoteAdd,
  Delete,
  FileCopy,
  InfoOutlined,
  FindInPage,
  WatchLater,
} from "@mui/icons-material";

interface TabPanelProps {
  children?: React.ReactNode;
  index: number;
  value: number;
  [key: string]: any;
}

// Declare global window properties for Membrane file system functions
declare global {
  interface Window {
    membrane_fs_read: (path: string) => Promise<{ status: string; data?: string; message?: string }>;
    membrane_fs_exists: (path: string) => Promise<{ status: string; data: boolean; message?: string }>;
    membrane_fs_delete: (path: string) => Promise<{ status: string; message?: string }>;
    membrane_fs_listDir: (path: string) => Promise<{ status: string; data: string[]; message?: string }>;
    membrane_fs_createDir: (path: string) => Promise<{ status: string; message?: string }>;
    membrane_fs_getInfo: (path: string) => Promise<{ 
      status: string; 
      data?: { 
        exists: boolean; 
        isDirectory: boolean; 
        isRegularFile: boolean; 
        size: number; 
        filename: string; 
        extension?: string; 
        lastModified?: string; 
        parent?: string; 
      }; 
      message?: string;
    }>;
    membrane_fs_copy: (sourcePath: string, destPath: string) => Promise<{ status: string; message?: string }>;
    membrane_fs_watch: (path: string, eventName: string) => Promise<{ status: string; message?: string }>;
    membrane_fs_readBinary: (path: string) => Promise<{ status: string; data: string; message?: string }>;
    membrane_fs_writeBinary: (path: string, base64Content: string) => Promise<{ status: string; message?: string }>;
    membrane_fs_createTemp: (prefix: string, extension: string) => Promise<{ status: string; data: string; message?: string }>;
    membrane_fs_save: (filePath: string, content: string) => Promise<{ status: string; message?: string; data?: any; }>;
  }
}

// Custom TabPanel component for tab content
function TabPanel({ children, value, index, ...other }: TabPanelProps) {
  return (
    <div
      role="tabpanel"
      hidden={value !== index}
      id={`fs-tabpanel-${index}`}
      aria-labelledby={`fs-tab-${index}`}
      {...other}
    >
      {value === index && <Box sx={{ p: 3 }}>{children}</Box>}
    </div>
  );
}

export const FileSystemDemo = () => {
  // State for feedback message
  const [feedback, setFeedback] = useState<{
    open: boolean;
    message: string;
    severity: "success" | "info" | "warning" | "error";
  }>({
    open: false,
    message: "",
    severity: "info",
  });

  // Tab state
  const [tabValue, setTabValue] = useState(0);

  // File paths and content state
  const [filePath, setFilePath] = useState("");
  const [directoryPath, setDirectoryPath] = useState("");
  const [fileContent, setFileContent] = useState("");
  const [sourcePath, setSourcePath] = useState("");
  const [destPath, setDestPath] = useState("");
  const [watchPath, setWatchPath] = useState("");
  const [binaryContent, setBinaryContent] = useState("");
  const [tempPrefix, setTempPrefix] = useState("membrane-");
  const [tempExtension, setTempExtension] = useState(".tmp");

  // Results state
  const [fileExists, setFileExists] = useState<boolean | null>(null);
  const [fileInfo, setFileInfo] = useState<{
    exists: boolean;
    isDirectory: boolean;
    isRegularFile: boolean;
    size: number;
    filename: string;
    extension?: string;
    lastModified?: string;
    parent?: string;
  } | null>(null);
  const [directoryContents, setDirectoryContents] = useState<string[]>([]);
  const [watchEvents, setWatchEvents] = useState<Array<{ type: any; path: any; time: string }>>([]);
  const [tempFilePath, setTempFilePath] = useState("");

  // Load available functions on mount
  useEffect(() => {
    // Setup a custom event listener for file watching
    const handleWatchEvent = (event: CustomEvent<{ type: any; path: any }>) => {
      const { type, path } = event.detail;
      setWatchEvents((prev) => [
        { type, path, time: new Date().toLocaleTimeString() },
        ...prev.slice(0, 9), // Keep only the last 10 events
      ]);
    };

    window.addEventListener("file-watch-event", handleWatchEvent as EventListener);

    return () => {
      window.removeEventListener("file-watch-event", handleWatchEvent as EventListener);
    };
  }, []);

  // Handle tab change
  const handleTabChange = (event: any, newValue: React.SetStateAction<number>) => {
    setTabValue(newValue);
  };

  // Shows a feedback message
  const showFeedback = (message: string, severity: "success" | "info" | "warning" | "error" = "success") => {
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

  // Basic File Operations
  const readFile = async () => {
    if (!filePath) {
      showFeedback("File path is required", "warning");
      return;
    }

    try {
      const result = await window.membrane_fs_read(filePath);

      if (result.status === "success") {
        setFileContent(result.data!);
        showFeedback(`File read successfully: ${filePath}`);
      } else {
        showFeedback(`Error: ${result.message}`, "error");
      }
    } catch (error: unknown) {
      const errorMessage =
        error instanceof Error ? error.message : "Unknown error occurred";
      showFeedback(`Error: ${errorMessage}`, "error");
    }
  };

  const saveFile = async () => {
    if (!filePath || !fileContent) {
      showFeedback("File path and content are required", "warning");
      return;
    }

    try {
      const result = await window.membrane_fs_save(filePath, fileContent);

      if (result.status === "success") {
        showFeedback(`File saved successfully: ${filePath}`);
      } else {
        showFeedback(`Error: ${result.message}`, "error");
      }
    } catch (error: unknown) {
      const errorMessage =
        error instanceof Error ? error.message : "Unknown error occurred";
      showFeedback(`Error: ${errorMessage}`, "error");
    }
  };

  const checkFileExists = async () => {
    if (!filePath) {
      showFeedback("File path is required", "warning");
      return;
    }

    try {
      const result = await window.membrane_fs_exists(filePath);

      if (result.status === "success") {
        setFileExists(result.data as boolean);
        showFeedback(
          result.data
            ? `File exists: ${filePath}`
            : `File does not exist: ${filePath}`,
          result.data ? "success" : "warning"
        );
      } else {
        showFeedback(`Error: ${result.message}`, "error");
      }
    } catch (error: unknown) {
      const errorMessage =
        error instanceof Error ? error.message : "Unknown error occurred";
      showFeedback(`Error: ${errorMessage}`, "error");
    }
  };

  const deleteFile = async () => {
    if (!filePath) {
      showFeedback("File path is required", "warning");
      return;
    }

    try {
      const result = await window.membrane_fs_delete(filePath);

      if (result.status === "success") {
        showFeedback(`File deleted successfully: ${filePath}`);
        setFileContent("");
      } else {
        showFeedback(`Error: ${result.message}`, "error");
      }
    } catch (error: unknown) {
      const errorMessage =
        error instanceof Error ? error.message : "Unknown error occurred";
      showFeedback(`Error: ${errorMessage}`, "error");
    }
  };

  // Directory Operations
  const listDirectory = async () => {
    if (!directoryPath) {
      showFeedback("Directory path is required", "warning");
      return;
    }

    try {
      const result = await window.membrane_fs_listDir(directoryPath);

      if (result.status === "success") {
        setDirectoryContents(result.data);
        showFeedback(`Listed ${result.data.length} items in directory`);
      } else {
        showFeedback(`Error: ${result.message}`, "error");
      }
    } catch (error: unknown) {
      const errorMessage =
        error instanceof Error ? error.message : "Unknown error occurred";
      showFeedback(`Error: ${errorMessage}`, "error");
    }
  };

  const createDirectory = async () => {
    if (!directoryPath) {
      showFeedback("Directory path is required", "warning");
      return;
    }

    try {
      const result = await window.membrane_fs_createDir(directoryPath);

      if (result.status === "success") {
        showFeedback(`Directory created: ${directoryPath}`);
      } else {
        showFeedback(`Error: ${result.message}`, "error");
      }
    } catch (error: unknown) {
      const errorMessage =
        error instanceof Error ? error.message : "Unknown error occurred";
      showFeedback(`Error: ${errorMessage}`, "error");
    }
  };

  // File Info and Copy
  const getFileInfo = async () => {
    if (!filePath) {
      showFeedback("File path is required", "warning");
      return;
    }

    try {
      const result = await window.membrane_fs_getInfo(filePath);

      if (result.status === "success" && result.data) {
        setFileInfo(result.data);
        showFeedback(`File information retrieved for: ${filePath}`);
      } else {
        showFeedback(`Error: ${result.message}`, "error");
      }
    } catch (error: unknown) {
      const errorMessage =
        error instanceof Error ? error.message : "Unknown error occurred";
      showFeedback(`Error: ${errorMessage}`, "error");
    }
  };

  const copyFile = async () => {
    if (!sourcePath || !destPath) {
      showFeedback("Source and destination paths are required", "warning");
      return;
    }

    try {
      const result = await window.membrane_fs_copy(sourcePath, destPath);

      if (result.status === "success") {
        showFeedback(`File copied from ${sourcePath} to ${destPath}`);
      } else {
        showFeedback(`Error: ${result.message}`, "error");
      }
    } catch (error: unknown) {
      const errorMessage =
        error instanceof Error ? error.message : "Unknown error occurred";
      showFeedback(`Error: ${errorMessage}`, "error");
    }
  };

  // Watch file
  const watchFile = async () => {
    if (!watchPath) {
      showFeedback("Path to watch is required", "warning");
      return;
    }

    try {
      const result = await window.membrane_fs_watch(
        watchPath,
        "file-watch-event"
      );

      if (result.status === "success") {
        showFeedback(`Started watching: ${watchPath}`);
      } else {
        showFeedback(`Error: ${result.message}`, "error");
      }
    } catch (error: unknown) {
      const errorMessage =
        error instanceof Error ? error.message : "Unknown error occurred";
      showFeedback(`Error: ${errorMessage}`, "error");
    }
  };

  // Binary file operations
  const readBinaryFile = async () => {
    if (!filePath) {
      showFeedback("File path is required", "warning");
      return;
    }

    try {
      const result = await window.membrane_fs_readBinary(filePath);

      if (result.status === "success") {
        // Get first 100 chars of base64 data as preview
        const preview =
          result.data.substring(0, 100) +
          (result.data.length > 100 ? "..." : "");
        setBinaryContent(preview);
        showFeedback(
          `Binary file read successfully: ${filePath} (${result.data.length} bytes)`
        );
      } else {
        showFeedback(`Error: ${result.message}`, "error");
      }
    } catch (error: unknown) {
      const errorMessage =
        error instanceof Error ? error.message : "Unknown error occurred";
      showFeedback(`Error: ${errorMessage}`, "error");
    }
  };

  const writeBinaryFile = async () => {
    if (!filePath || !binaryContent) {
      showFeedback("File path and binary content are required", "warning");
      return;
    }

    try {
      const result = await window.membrane_fs_writeBinary(
        filePath,
        binaryContent
      );

      if (result.status === "success") {
        showFeedback(`Binary file written successfully: ${filePath}`);
      } else {
        showFeedback(`Error: ${result.message}`, "error");
      }
    } catch (error: unknown) {
      const errorMessage =
        error instanceof Error ? error.message : "Unknown error occurred";
      showFeedback(`Error: ${errorMessage}`, "error");
    }
  };

  // Temporary file operations
  const createTempFile = async () => {
    try {
      const result = await window.membrane_fs_createTemp(
        tempPrefix,
        tempExtension
      );

      if (result.status === "success") {
        setTempFilePath(result.data);
        showFeedback(`Temporary file created: ${result.data}`);
      } else {
        showFeedback(`Error: ${result.message}`, "error");
      }
    } catch (error: unknown) {
      const errorMessage =
        error instanceof Error ? error.message : "Unknown error occurred";
      showFeedback(`Error: ${errorMessage}`, "error");
    }
  };

  // UI Rendering Functions
  const renderBasicFileOps = () => (
    <TabPanel value={tabValue} index={0}>
      <Typography variant="h6" gutterBottom>
        Basic File Operations
      </Typography>
      <Grid container spacing={2}>
        <Grid item xs={12}>
          <TextField
            fullWidth
            label="File Path"
            variant="outlined"
            value={filePath}
            onChange={(e) => setFilePath(e.target.value)}
          />
        </Grid>
        <Grid item xs={12}>
          <TextField
            fullWidth
            label="File Content"
            variant="outlined"
            multiline
            rows={6}
            value={fileContent}
            onChange={(e) => setFileContent(e.target.value)}
          />
        </Grid>
        <Grid item container spacing={1}>
          <Grid item>
            <Button
              variant="contained"
              color="primary"
              startIcon={<ContentPaste />}
              onClick={readFile}
            >
              Read File
            </Button>
          </Grid>
          <Grid item>
            <Button
              variant="contained"
              color="secondary"
              startIcon={<Save />}
              onClick={saveFile}
            >
              Save File
            </Button>
          </Grid>
          <Grid item>
            <Button
              variant="outlined"
              onClick={checkFileExists}
              startIcon={<FindInPage />}
            >
              Check Exists
            </Button>
          </Grid>
          <Grid item>
            <Button
              variant="outlined"
              color="error"
              startIcon={<Delete />}
              onClick={deleteFile}
            >
              Delete File
            </Button>
          </Grid>
        </Grid>

        {fileExists !== null && (
          <Grid item xs={12}>
            <Alert severity={fileExists ? "success" : "warning"}>
              {fileExists
                ? `The file "${filePath}" exists on disk.`
                : `The file "${filePath}" does not exist.`}
            </Alert>
          </Grid>
        )}
      </Grid>
    </TabPanel>
  );

  const renderDirectoryOps = () => (
    <TabPanel value={tabValue} index={1}>
      <Typography variant="h6" gutterBottom>
        Directory Operations
      </Typography>
      <Grid container spacing={2}>
        <Grid item xs={12}>
          <TextField
            fullWidth
            label="Directory Path"
            variant="outlined"
            value={directoryPath}
            onChange={(e) => setDirectoryPath(e.target.value)}
          />
        </Grid>
        <Grid item container spacing={1}>
          <Grid item>
            <Button
              variant="contained"
              color="primary"
              startIcon={<ContentPaste />}
              onClick={listDirectory}
            >
              List Directory
            </Button>
          </Grid>
          <Grid item>
            <Button
              variant="contained"
              color="secondary"
              startIcon={<CreateNewFolder />}
              onClick={createDirectory}
            >
              Create Directory
            </Button>
          </Grid>
        </Grid>

        {directoryContents.length > 0 && (
          <Grid item xs={12}>
            <Card variant="outlined" sx={{ mt: 2 }}>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Directory Contents
                </Typography>
                <List>
                  {directoryContents.map((item, index) => (
                    <ListItem key={index}>
                      <ListItemText primary={item} />
                    </ListItem>
                  ))}
                </List>
              </CardContent>
            </Card>
          </Grid>
        )}
      </Grid>
    </TabPanel>
  );

  const renderFileInfoAndCopy = () => (
    <TabPanel value={tabValue} index={2}>
      <Typography variant="h6" gutterBottom>
        File Information
      </Typography>
      <Grid container spacing={2}>
        <Grid item xs={12}>
          <TextField
            fullWidth
            label="File Path"
            variant="outlined"
            value={filePath}
            onChange={(e) => setFilePath(e.target.value)}
          />
        </Grid>
        <Grid item>
          <Button
            variant="contained"
            color="primary"
            startIcon={<InfoOutlined />}
            onClick={getFileInfo}
          >
            Get File Info
          </Button>
        </Grid>

        {fileInfo && (
          <Grid item xs={12}>
            <Card variant="outlined" sx={{ mt: 2 }}>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  File Information
                </Typography>
                <Grid container spacing={1}>
                  <Grid item xs={6}>
                    <Typography variant="body1">
                      Type:{" "}
                      {fileInfo.isDirectory
                        ? "Directory"
                        : fileInfo.isRegularFile
                        ? "File"
                        : "Other"}
                    </Typography>
                  </Grid>
                  <Grid item xs={6}>
                    <Typography variant="body1">
                      Size: {fileInfo.size} bytes
                    </Typography>
                  </Grid>
                  <Grid item xs={6}>
                    <Typography variant="body1">
                      Exists: {fileInfo.exists ? "Yes" : "No"}
                    </Typography>
                  </Grid>
                  <Grid item xs={6}>
                    <Typography variant="body1">
                      Filename: {fileInfo.filename}
                    </Typography>
                  </Grid>
                  {fileInfo.extension && (
                    <Grid item xs={6}>
                      <Typography variant="body1">
                        Extension: {fileInfo.extension}
                      </Typography>
                    </Grid>
                  )}
                  {fileInfo.lastModified && (
                    <Grid item xs={6}>
                      <Typography variant="body1">
                        Last Modified: {fileInfo.lastModified}
                      </Typography>
                    </Grid>
                  )}
                  {fileInfo.parent && (
                    <Grid item xs={12}>
                      <Typography variant="body1">
                        Parent Directory: {fileInfo.parent}
                      </Typography>
                    </Grid>
                  )}
                </Grid>
              </CardContent>
            </Card>
          </Grid>
        )}

        <Grid item xs={12}>
          <Divider sx={{ my: 2 }} />
        </Grid>

        <Grid item xs={12}>
          <Typography variant="h6" gutterBottom>
            Copy File or Directory
          </Typography>
        </Grid>

        <Grid item xs={12} sm={6}>
          <TextField
            fullWidth
            label="Source Path"
            variant="outlined"
            value={sourcePath}
            onChange={(e) => setSourcePath(e.target.value)}
          />
        </Grid>
        <Grid item xs={12} sm={6}>
          <TextField
            fullWidth
            label="Destination Path"
            variant="outlined"
            value={destPath}
            onChange={(e) => setDestPath(e.target.value)}
          />
        </Grid>
        <Grid item>
          <Button
            variant="contained"
            color="secondary"
            startIcon={<FileCopy />}
            onClick={copyFile}
          >
            Copy
          </Button>
        </Grid>
      </Grid>
    </TabPanel>
  );

  const renderWatchFile = () => (
    <TabPanel value={tabValue} index={3}>
      <Typography variant="h6" gutterBottom>
        Watch File or Directory
      </Typography>
      <Grid container spacing={2}>
        <Grid item xs={12}>
          <TextField
            fullWidth
            label="Path to Watch"
            variant="outlined"
            value={watchPath}
            onChange={(e) => setWatchPath(e.target.value)}
          />
        </Grid>
        <Grid item>
          <Button
            variant="contained"
            color="primary"
            startIcon={<WatchLater />}
            onClick={watchFile}
          >
            Start Watching
          </Button>
        </Grid>

        {watchEvents.length > 0 && (
          <Grid item xs={12}>
            <Card variant="outlined" sx={{ mt: 2 }}>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Watch Events
                </Typography>
                <div style={{ maxHeight: "200px", overflow: "auto" }}>
                  <List>
                    {watchEvents.map((event, index) => (
                      <ListItem
                        key={index}
                        divider={index < watchEvents.length - 1}
                      >
                        <ListItemText
                          primary={`${event.type}: ${event.path}`}
                          secondary={event.time}
                        />
                      </ListItem>
                    ))}
                  </List>
                </div>
              </CardContent>
            </Card>
          </Grid>
        )}

        <Grid item xs={12}>
          <Divider sx={{ my: 2 }} />
        </Grid>

        <Grid item xs={12}>
          <Typography variant="h6" gutterBottom>
            Create Temporary File
          </Typography>
        </Grid>

        <Grid item xs={12} sm={6}>
          <TextField
            fullWidth
            label="Prefix"
            variant="outlined"
            value={tempPrefix}
            onChange={(e) => setTempPrefix(e.target.value)}
          />
        </Grid>
        <Grid item xs={12} sm={6}>
          <TextField
            fullWidth
            label="Extension"
            variant="outlined"
            value={tempExtension}
            onChange={(e) => setTempExtension(e.target.value)}
          />
        </Grid>
        <Grid item>
          <Button
            variant="contained"
            color="secondary"
            onClick={createTempFile}
          >
            Create Temp File
          </Button>
        </Grid>

        {tempFilePath && (
          <Grid item xs={12}>
            <Alert severity="success" sx={{ mt: 2 }}>
              <Typography>Temporary file created at:</Typography>
              <Typography
                variant="body2"
                sx={{
                  wordBreak: "break-all",
                  fontFamily: "monospace",
                  mt: 1,
                }}
              >
                {tempFilePath}
              </Typography>
            </Alert>
          </Grid>
        )}
      </Grid>
    </TabPanel>
  );

  const renderBinaryOps = () => (
    <TabPanel value={tabValue} index={4}>
      <Typography variant="h6" gutterBottom>
        Binary File Operations
      </Typography>
      <Grid container spacing={2}>
        <Grid item xs={12}>
          <TextField
            fullWidth
            label="File Path"
            variant="outlined"
            value={filePath}
            onChange={(e) => setFilePath(e.target.value)}
          />
        </Grid>
        <Grid item xs={12}>
          <TextField
            fullWidth
            label="Binary Content (Base64)"
            variant="outlined"
            multiline
            rows={4}
            value={binaryContent}
            onChange={(e) => setBinaryContent(e.target.value)}
            helperText="For binary files, content is encoded/decoded as Base64"
          />
        </Grid>
        <Grid item container spacing={1}>
          <Grid item>
            <Button
              variant="contained"
              color="primary"
              onClick={readBinaryFile}
            >
              Read Binary File
            </Button>
          </Grid>
          <Grid item>
            <Button
              variant="contained"
              color="secondary"
              onClick={writeBinaryFile}
            >
              Write Binary File
            </Button>
          </Grid>
        </Grid>
      </Grid>
    </TabPanel>
  );

  return (
    <>
      <CssBaseline />
      <Container maxWidth="md" sx={{ py: 4 }}>
        <Typography variant="h4" gutterBottom align="center">
          File System Functions Demo
        </Typography>

        <Paper sx={{ mt: 3 }}>
          <Tabs
            value={tabValue}
            onChange={handleTabChange}
            variant="scrollable"
            scrollButtons="auto"
          >
            <Tab label="Basic File Ops" />
            <Tab label="Directories" />
            <Tab label="Info & Copy" />
            <Tab label="Watch & Temp" />
            <Tab label="Binary Files" />
          </Tabs>

          {renderBasicFileOps()}
          {renderDirectoryOps()}
          {renderFileInfoAndCopy()}
          {renderWatchFile()}
          {renderBinaryOps()}
        </Paper>
      </Container>

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
    </>
  );
};

export default FileSystemDemo;
