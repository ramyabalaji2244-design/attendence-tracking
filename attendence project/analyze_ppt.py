#!/usr/bin/env python3
"""
Script to analyze PowerPoint presentation and extract content
"""
import zipfile
import xml.etree.ElementTree as ET
import os
import sys

def analyze_pptx(pptx_path):
    """Extract and analyze PPTX content"""
    
    if not os.path.exists(pptx_path):
        print(f"Error: File not found - {pptx_path}")
        return None
    
    try:
        # Open PPTX as ZIP
        with zipfile.ZipFile(pptx_path, 'r') as zip_ref:
            # List all files
            print("=== PPTX File Contents ===")
            print(zip_ref.namelist())
            print("\n")
            
            # Extract and read presentation.xml to understand structure
            if 'ppt/presentation.xml' in zip_ref.namelist():
                print("=== Presentation Structure Found ===\n")
            
            # Look for slide content
            slides = [f for f in zip_ref.namelist() if f.startswith('ppt/slides/slide') and f.endswith('.xml')]
            print(f"Found {len(slides)} slides\n")
            
            # Extract text from each slide
            for slide_file in sorted(slides):
                print(f"\n{'='*60}")
                print(f"Content from: {slide_file}")
                print('='*60)
                
                try:
                    slide_content = zip_ref.read(slide_file).decode('utf-8')
                    
                    # Parse XML
                    root = ET.fromstring(slide_content)
                    
                    # Extract all text
                    ns = {
                        'a': 'http://schemas.openxmlformats.org/drawingml/2006/main',
                        'p': 'http://schemas.openxmlformats.org/presentationml/2006/main',
                        'r': 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'
                    }
                    
                    # Find all text elements
                    for t_elem in root.findall('.//a:t', ns):
                        if t_elem.text:
                            print(f"  • {t_elem.text}")
                    
                except Exception as e:
                    print(f"Error processing slide: {e}")
            
            # Also check for notes
            notes_files = [f for f in zip_ref.namelist() if f.startswith('ppt/notesSlides/') and f.endswith('.xml')]
            if notes_files:
                print(f"\n\nFound {len(notes_files)} note files\n")
                for note_file in sorted(notes_files):
                    print(f"\nNotes from: {note_file}")
                    try:
                        note_content = zip_ref.read(note_file).decode('utf-8')
                        root = ET.fromstring(note_content)
                        ns = {'a': 'http://schemas.openxmlformats.org/drawingml/2006/main'}
                        for t_elem in root.findall('.//a:t', ns):
                            if t_elem.text:
                                print(f"  • {t_elem.text}")
                    except:
                        pass
        
        return True
        
    except Exception as e:
        print(f"Error analyzing PPTX: {e}")
        return False

if __name__ == '__main__':
    # Find the PPTX file
    ppt_files = [f for f in os.listdir('.') if f.endswith('.pptx')]
    
    if ppt_files:
        print(f"Found PPTX file: {ppt_files[0]}\n")
        analyze_pptx(ppt_files[0])
    else:
        print("No PPTX files found in current directory")
        print(f"Current directory: {os.getcwd()}")
        print(f"Files: {os.listdir('.')}")
