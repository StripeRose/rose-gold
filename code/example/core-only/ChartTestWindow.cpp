// Filter "Chart"
#include "stdafx.hpp"
#include "ChartTestWindow.hpp"
#include "ChartData.hpp"
#include "ChartTrack.hpp"

#include "Common_Diagnostics.hpp"

#include "Editor_FileDialog.hpp"
#include "Editor_GUI.hpp"

#if IS_IMGUI_ENABLED
#define NOTE_SILVER       IM_COL32(217, 226, 228, 255)
#define NOTE_BLACK        IM_COL32(10, 10, 10, 255)

#define NOTE_HOPO         IM_COL32(250, 250, 200, 255)
#define NOTE_STARPOWER    IM_COL32(83, 254, 254, 255)

#define NOTE_OPEN         IM_COL32(150, 65, 197, 255)

#define NOTE_GREEN        IM_COL32(0, 240, 0, 255)
#define NOTE_RED          IM_COL32(240, 120, 120, 255)
#define NOTE_YELLOW       IM_COL32(240, 240, 70, 255)
#define NOTE_BLUE         IM_COL32(105, 184, 240, 255)
#define NOTE_ORANGE       IM_COL32(240, 180, 45, 255)

#define NOTE_TAP_GREEN    IM_COL32(0, 250, 0, 255)
#define NOTE_TAP_RED      IM_COL32(250, 0, 0, 255)
#define NOTE_TAP_YELLOW   IM_COL32(250, 250, 0, 255)
#define NOTE_TAP_BLUE     IM_COL32(0, 135, 250, 255)
#define NOTE_TAP_ORANGE   IM_COL32(255, 180, 0, 255)

#define NOTE_RADIUS_SP 13
#define NOTE_RADIUS_OUTLINE 11
#define NOTE_RADIUS 10
#define NOTE_RADIUS_TAP 5
#define NOTE_RADIUS_TIP 3

#define NOTE_RADIUS_OPEN_HOPO 9
#define NOTE_RADIUS_OPEN_OUTLINE 7
#define NOTE_RADIUS_OPEN 5
#endif

ChartTestWindow::ChartTestWindow()
	: myLookAhead(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::seconds(5)))
{
}

void ChartTestWindow::ImGui()
{
#if IS_IMGUI_ENABLED
	std::string title = std::format("Chart - {}###ChartTestWindow", myCurrentSong);
	if (ImGui::Begin(title.c_str()))
	{
		switch (myState)
		{
		case State::SongList:
			ImGui_ChartList();
			break;
		case State::Player:
			ImGui_Player();
			break;
		}
	}
	ImGui::End();
#endif
}

void ChartTestWindow::ImGui_ChartList()
{
#if IS_IMGUI_ENABLED
	ImGui::TextUnformatted(mySongsDirectory.string().c_str());
	ImGui::SameLine();

	if (ImGui::Button("..."))
	{
		RoseGold::Editor::FolderBrowserDialog pickFolder;
		auto pickedFolder = pickFolder.GetSingle();
		if (pickedFolder.has_value())
			mySongsDirectory = pickedFolder.value();
	}

	ImGui::NewLine();

	if (ImGui::Button("Refresh"))
		RefreshSongList();

	if (ImGui::BeginTable("Song table", 3))
	{
		ImGui::TableSetupColumn("Info");
		ImGui::TableSetupColumn("Details");
		ImGui::TableSetupColumn("Actions");
		ImGui::TableHeadersRow();

		for (const auto& it : myChartInfos)
		{
			ImGui::TableNextRow();
			ImGui::PushID(it.first.c_str());

			const ChartInfo::SongInfo& songInfo = it.second->GetSongInfo();

			ImGui::TableNextColumn();
			ImGui::TextUnformatted(songInfo.Title.c_str());
			ImGui::Text("by %s", songInfo.Artist.c_str());
			ImGui::Text("%s (%i)", songInfo.Album.c_str(), songInfo.Year);
			ImGui::TextUnformatted(std::format("{:%M:%S}", std::chrono::round<std::chrono::seconds>(songInfo.SongLength)).c_str());

			ImGui::TableNextColumn();

			auto difficultyWidget = [&](ChartTrackType aTrack, const char* aTitle)
				{
					const int difficulty = it.second->GetDifficulty(aTrack);
					if (difficulty == -1)
						return;

					const float max = static_cast<float>(ChartInfo::MaxDifficulty);
					const float current = std::clamp(static_cast<float>(difficulty), 0.f, max);
					ImGui::ProgressBar(current / max, { 0, 0 }, aTitle);
				};

			difficultyWidget(ChartTrackType::LeadGuitar, "Lead guitar");
			difficultyWidget(ChartTrackType::RhythmGuitar, "Rhythm guitar");
			difficultyWidget(ChartTrackType::BassGuitar, "Bass guitar");
			difficultyWidget(ChartTrackType::Drums, "Drums");
			difficultyWidget(ChartTrackType::Vocal_Main, "Vocals");
			difficultyWidget(ChartTrackType::Vocal_Harmony, "Harmony vocals");

			ImGui::TableNextColumn();

			if (ImGui::Button("Select"))
				SelectSong(it.first);

			ImGui::PopID();
		}

		ImGui::EndTable();
	}
#endif
}

void ChartTestWindow::ImGui_Player()
{
#if IS_IMGUI_ENABLED
	myChartPlayer.Update();
	if (ImGui::Button("Back to song list"))
		ReturnToSongList();

	ImGui_Player_PlayControls();

	{
		int lookaheadMicroseconds = static_cast<int>(myLookAhead.count());
		const auto lookaheadSeconds = std::chrono::duration_cast<std::chrono::seconds>(myLookAhead);
		if (ImGui::SliderInt("Look ahead", &lookaheadMicroseconds, 0, static_cast<int>(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::seconds(15)).count()), std::format("{} seconds", lookaheadSeconds.count()).c_str()))
			myLookAhead = std::chrono::microseconds(lookaheadMicroseconds);
	}

	for (const auto& track : myChartData.GetTracks())
		ImGui_Player_Track(*track.second);
#endif
}

void ChartTestWindow::ImGui_Player_PlayControls()
{
#if IS_IMGUI_ENABLED
	ImGui::Text("BPM: %f", myChartData.GetBPMAt(myChartPlayer.GetPlayhead()));

	const ChartPlayer::State playerState = myChartPlayer.GetState();
	ImGui::BeginDisabled(playerState == ChartPlayer::State::Seeking);

	ImGui::BeginDisabled(playerState == ChartPlayer::State::Playing);
	if (ImGui::Button("Play"))
		myChartPlayer.Play();
	ImGui::EndDisabled();

	ImGui::SameLine();

	ImGui::BeginDisabled(playerState == ChartPlayer::State::Paused);
	if (ImGui::Button("Pause"))
		myChartPlayer.Pause();
	ImGui::EndDisabled();

	ImGui::SameLine();

	ImGui::BeginDisabled(playerState == ChartPlayer::State::Stopped);
	if (ImGui::Button("Stop"))
		myChartPlayer.Stop();
	ImGui::EndDisabled();

	ImGui::EndDisabled();

	const auto playhead = myChartPlayer.GetPlayhead();
	const auto duration = myChartData.GetDuration();
	const auto playheadSeconds = std::chrono::round<std::chrono::seconds>(playhead);
	const auto durationSeconds = std::chrono::round<std::chrono::seconds>(duration);
	int playheadMicroseconds = static_cast<int>(playhead.count());
	if (ImGui::SliderInt("###SeekBar", &playheadMicroseconds, 0, static_cast<int>(duration.count()), std::format("{:%M:%S} / {:%M:%S}", playheadSeconds, durationSeconds).c_str()))
	{
		if (playerState == ChartPlayer::State::Stopped)
			myChartPlayer.Pause();
		myChartPlayer.Seek(std::chrono::microseconds(playheadMicroseconds));
	}

#endif
}

void ChartTestWindow::ImGui_Player_Track(ChartTrack& aTrack)
{
#if IS_IMGUI_ENABLED
	const char* trackTypeString = nullptr;
	switch (aTrack.GetType())
	{
	case ChartTrackType::Drums:
		trackTypeString = "Drums";
		break;
	case ChartTrackType::LeadGuitar:
		trackTypeString = "Lead guitar";
		break;
	case ChartTrackType::RhythmGuitar:
		trackTypeString = "Rhythm guitar";
		break;
	case ChartTrackType::BassGuitar:
		trackTypeString = "Bass guitar";
		break;
	case ChartTrackType::Vocal_Main:
		trackTypeString = "Vocals";
		break;
	case ChartTrackType::Vocal_Harmony:
		trackTypeString = "Harmony vocals";
		break;
	}

	RoseGold::Debug::Assert(trackTypeString, "Track rendering needs a valid track.");

	if (!ImGui::TreeNode(trackTypeString))
		return;

	{
		int currentDifficulty = static_cast<int>(myTrackSettings[aTrack.GetType()].Difficulty);
		ImGui::Combo("Difficulty", &currentDifficulty, "Easy\0Medium\0Hard\0Expert\0\0");
		myTrackSettings[aTrack.GetType()].Difficulty = ChartTrackDifficulty(currentDifficulty);
	}

	ImGui::Checkbox("Show open notes", &myTrackSettings[aTrack.GetType()].ShowOpen);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(34, 34, 34, 255));
	const bool isVisible = ImGui::BeginChild("track", ImVec2(0, 150.f), ImGuiChildFlags_Border, ImGuiWindowFlags_NoMove);
	ImGui::PopStyleColor();
	ImGui::PopStyleVar();

	if (isVisible)
	{
		ImVec2 canvasTopLeft = ImGui::GetCursorScreenPos();
		ImVec2 canvasSize = ImGui::GetContentRegionAvail();

		if (canvasSize.x < 50.0f) canvasSize.x = 50.0f;
		if (canvasSize.y < 50.0f) canvasSize.y = 50.0f;
		ImVec2 canvasBottomRight = ImVec2(canvasTopLeft.x + canvasSize.x, canvasTopLeft.y + canvasSize.y);

		//ImGuiIO& io = ImGui::GetIO();
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		drawList->AddRectFilled(canvasTopLeft, canvasBottomRight, IM_COL32(50, 50, 50, 255));

		switch (aTrack.GetType())
		{
		case ChartTrackType::Drums:
			break;
		case ChartTrackType::LeadGuitar:
		case ChartTrackType::RhythmGuitar:
		case ChartTrackType::BassGuitar:
			ImGui_Player_Track(
				static_cast<ChartGuitarTrack&>(aTrack),
				{ canvasTopLeft.x + 1, canvasTopLeft.y + 1 },
				{ canvasSize.x - 2, canvasSize.y - 2 });
			break;
		case ChartTrackType::Vocal_Main:
		case ChartTrackType::Vocal_Harmony:
			break;
		}

		drawList->AddRect(canvasTopLeft, canvasBottomRight, IM_COL32(255, 255, 255, 255));
	}

	ImGui::EndChild();

	ImGui::TreePop();
#endif
}

void ChartTestWindow::ImGui_Player_Track(ChartGuitarTrack& aTrack, RoseGold::Math::Vector2 aPoint, RoseGold::Math::Vector2 aSize)
{
#if IS_IMGUI_ENABLED
	const ImVec2 canvasTopLeft(aPoint.X, aPoint.Y);
	const ImVec2 canvasSize(aSize.X, aSize.Y);
	const ImVec2 canvasBottomRight(aPoint.X + aSize.X, aPoint.Y + aSize.Y);

	ImDrawList* drawList = ImGui::GetWindowDrawList();

	float laneY[5];

	auto drawGuitarLane = [&](ChartGuitarTrack::Lane aLane, ImColor aColor) {

		const float laneHeight = (canvasSize.y / 5);
		ImVec2 topLeft = canvasTopLeft;
		topLeft.y += laneHeight * static_cast<int>(aLane);
		ImVec2 bottomRight = canvasBottomRight;
		bottomRight.y = topLeft.y + laneHeight;

		const ImVec2 centerLeft(topLeft.x, std::lerp(topLeft.y, bottomRight.y, 0.5f));
		const ImVec2 centerRight(bottomRight.x, std::lerp(topLeft.y, bottomRight.y, 0.5f));
		laneY[static_cast<int>(aLane)] = centerLeft.y;
		drawList->AddLine(centerLeft, centerRight, aColor, 2.f);
		};

	drawGuitarLane(ChartGuitarTrack::Lane::Green, IM_COL32(20, 100, 20, 255));
	drawGuitarLane(ChartGuitarTrack::Lane::Red, IM_COL32(100, 20, 20, 255));
	drawGuitarLane(ChartGuitarTrack::Lane::Yellow, IM_COL32(100, 100, 20, 255));
	drawGuitarLane(ChartGuitarTrack::Lane::Blue, IM_COL32(20, 20, 100, 255));
	drawGuitarLane(ChartGuitarTrack::Lane::Orange, IM_COL32(100, 70, 20, 255));

	drawList->AddLine(
		ImVec2(canvasTopLeft.x + 20.f, canvasTopLeft.y),
		ImVec2(canvasTopLeft.x + 20.f, canvasBottomRight.y),
		IM_COL32(100, 100, 100, 255), 2.f);

	// Draw notes.
	const TrackSettings& trackSettings = myTrackSettings.at(aTrack.GetType());
	if (!aTrack.GetNoteRanges().contains(trackSettings.Difficulty))
		return;

	const std::vector<ChartGuitarTrack::NoteRange>& difficultyNotes = aTrack.GetNoteRanges().at(trackSettings.Difficulty);
	for (const ChartGuitarTrack::NoteRange& note : difficultyNotes)
	{
		const float noteXStart = ImGui_GetNoteXFraction(note.Start);
		const float noteXEnd = ImGui_GetNoteXFraction(note.End);
		if (noteXEnd < -2.f || 1.2f < noteXStart)
			continue;

		const float noteStart = std::lerp(canvasTopLeft.x + 20.f, canvasBottomRight.x, noteXStart);
		const float noteEnd = std::lerp(canvasTopLeft.x + 20.f, canvasBottomRight.x, noteXEnd);

		if (trackSettings.ShowOpen && note.CanBeOpen)
			ImGui_OpenNote(note.Type != ChartGuitarTrack::NoteType::Strum, noteStart, noteEnd, laneY[0], laneY[4]);
		else
			ImGui_Note(note, noteStart, noteEnd, laneY[static_cast<int>(note.Lane)]);
	}

#endif
}

void ChartTestWindow::ImGui_Note(const ChartGuitarTrack::NoteRange& aNote, float aStartX, float anEndX, float aLaneY)
{
	ImVec2 startPosition(aStartX, aLaneY);
	ImVec2 endPosition(anEndX, aLaneY);

	ImDrawList* drawList = ImGui::GetWindowDrawList();

	ImU32 strumColor = NOTE_BLACK;
	switch (aNote.Lane)
	{
	case ChartGuitarTrack::Lane::Green:
		strumColor = NOTE_GREEN;
		break;
	case ChartGuitarTrack::Lane::Red:
		strumColor = NOTE_RED;
		break;
	case ChartGuitarTrack::Lane::Yellow:
		strumColor = NOTE_YELLOW;
		break;
	case ChartGuitarTrack::Lane::Blue:
		strumColor = NOTE_BLUE;
		break;
	case ChartGuitarTrack::Lane::Orange:
		strumColor = NOTE_ORANGE;
		break;
	}

	drawList->AddRectFilled(
		ImVec2(startPosition.x, startPosition.y - 2.f),
		ImVec2(endPosition.x, endPosition.y + 2.f),
		strumColor,
		2.f
	);

	const bool isStarPower = false;
	drawList->AddCircleFilled(startPosition, NOTE_RADIUS_SP, isStarPower ? NOTE_STARPOWER : NOTE_SILVER);

	switch (aNote.Type)
	{
	case ChartGuitarTrack::NoteType::Strum:
	case ChartGuitarTrack::NoteType::HOPO:
	{
		drawList->AddCircleFilled(startPosition, NOTE_RADIUS, strumColor);
		break;
	}
	case ChartGuitarTrack::NoteType::Tap:
	{
		ImU32 noteColor = NOTE_BLACK;
		switch (aNote.Lane)
		{
		case ChartGuitarTrack::Lane::Green:
			noteColor = NOTE_TAP_GREEN;
			break;
		case ChartGuitarTrack::Lane::Red:
			noteColor = NOTE_TAP_RED;
			break;
		case ChartGuitarTrack::Lane::Yellow:
			noteColor = NOTE_TAP_YELLOW;
			break;
		case ChartGuitarTrack::Lane::Blue:
			noteColor = NOTE_TAP_BLUE;
			break;
		case ChartGuitarTrack::Lane::Orange:
			noteColor = NOTE_TAP_ORANGE;
			break;
		}
		drawList->AddCircle(startPosition, NOTE_RADIUS, noteColor);
		drawList->AddCircle(startPosition, NOTE_RADIUS - 1, noteColor);
		break;
	}
	}

	drawList->AddCircle(startPosition, NOTE_RADIUS_OUTLINE, NOTE_BLACK);

	drawList->AddCircleFilled(
		startPosition,
		NOTE_RADIUS_TAP,
		aNote.Type == ChartGuitarTrack::NoteType::HOPO ? NOTE_HOPO : NOTE_BLACK
	);

	drawList->AddCircleFilled(startPosition, NOTE_RADIUS_TIP, NOTE_SILVER);
}

void ChartTestWindow::ImGui_OpenNote(bool isHOPO, float aNoteStart, float aNoteEnd, float aTopLane, float aBottomLane)
{
	ImDrawList* drawList = ImGui::GetWindowDrawList();

	drawList->AddRectFilled(
		{ aNoteStart, aTopLane },
		{ aNoteEnd, aBottomLane },
		(NOTE_OPEN) & IM_COL32(255, 255, 255, 100),
		NOTE_RADIUS_OPEN_HOPO
	);

	drawList->AddRect(
		{ aNoteStart, aTopLane },
		{ aNoteEnd, aBottomLane },
		NOTE_OPEN,
		NOTE_RADIUS_OPEN_HOPO
	);

	drawList->AddRectFilled(
		{ aNoteStart - NOTE_RADIUS_OPEN_HOPO, aTopLane - NOTE_RADIUS_OPEN_HOPO },
		{ aNoteStart + NOTE_RADIUS_OPEN_HOPO, aBottomLane + NOTE_RADIUS_OPEN_HOPO },
		isHOPO ? NOTE_HOPO : NOTE_SILVER,
		NOTE_RADIUS_OPEN_HOPO);

	drawList->AddRectFilled(
		{ aNoteStart - NOTE_RADIUS_OPEN_OUTLINE, aTopLane - NOTE_RADIUS_OPEN_OUTLINE },
		{ aNoteStart + NOTE_RADIUS_OPEN_OUTLINE, aBottomLane + NOTE_RADIUS_OPEN_OUTLINE },
		isHOPO ? NOTE_HOPO : NOTE_BLACK,
		NOTE_RADIUS_OPEN_OUTLINE);

	const bool isStarPower = false;
	drawList->AddRectFilled(
		{ aNoteStart - NOTE_RADIUS_OPEN, aTopLane - NOTE_RADIUS_OPEN },
		{ aNoteStart + NOTE_RADIUS_OPEN, aBottomLane + NOTE_RADIUS_OPEN },
		isStarPower ? NOTE_STARPOWER : NOTE_OPEN,
		NOTE_RADIUS_OPEN);
}

float ChartTestWindow::ImGui_GetNoteXFraction(std::chrono::microseconds aTime) const
{
	aTime -= myChartPlayer.GetPlayhead();
	return static_cast<float>(aTime.count()) / static_cast<float>(myLookAhead.count());
}

void ChartTestWindow::RefreshSongList()
{
	if (!std::filesystem::exists(mySongsDirectory) || !std::filesystem::is_directory(mySongsDirectory))
		return;

	myChartInfos.clear();

	for (const auto& entry : std::filesystem::directory_iterator(mySongsDirectory))
	{
		const std::filesystem::path songIni = entry.path() / "song.ini";

		if (!std::filesystem::exists(songIni))
			continue;

		std::unique_ptr<ChartInfo> chartInfo = std::make_unique<ChartInfo>();
		chartInfo->Load(songIni);
		myChartInfos[songIni] = std::move(chartInfo);
	}
}

void ChartTestWindow::SelectSong(const std::filesystem::path& aSong)
{
	const std::unique_ptr<ChartInfo>& chart = myChartInfos.at(aSong);
	myCurrentSong = chart->GetSongInfo().Title;
	myChartData.LoadMidi(aSong.parent_path() / "notes.mid");
	myChartPlayer.SetChartData(myChartData);
	myState = State::Player;
}

void ChartTestWindow::ReturnToSongList()
{
	myCurrentSong.clear();
	myChartPlayer.Stop();
	myState = State::SongList;
}
